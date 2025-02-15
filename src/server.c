#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "server.h"
#include "parser.h"
#include "renderer.h"
#include "auth.h"
#include "utils.h"
#include "site_manager.h"

#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;
    time_t connect_time = time(NULL);

    char client_ip[16];
    get_client_ip(client_socket, client_ip);
    printf("\033[1;32m[INFO]\033[0m New connection from %s\n", client_ip);

    if (!authenticate_client(client_socket)) {
        printf("\033[1;31m[ERROR]\033[0m Authentication failed for %s\n", client_ip);
        close(client_socket);
        return;
    }

    printf("\033[1;32m[INFO]\033[0m Client %s authenticated successfully\n", client_ip);

    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytes_read] = '\0';

        if (strncmp(buffer, "open ", 5) == 0) {
            char *url = buffer + 5;
            url[strcspn(url, "\n")] = 0;
            open_spp_file(client_socket, url);
            printf("\033[1;34m[INFO]\033[0m Client %s opened file: %s\n", client_ip, url);
        } else if (strncmp(buffer, "list", 4) == 0) {
            list_spp_sites(client_socket);
            printf("\033[1;34m[INFO]\033[0m Client %s requested site list\n", client_ip);
        } else if (strncmp(buffer, "stats", 5) == 0) {
            show_stats(client_socket, connect_time);
            printf("\033[1;34m[INFO]\033[0m Client %s requested stats\n", client_ip);
        } else if (strncmp(buffer, "search ", 7) == 0) {
            char *keyword = buffer + 7;
            keyword[strcspn(keyword, "\n")] = 0;
            search_files(client_socket, keyword);
            printf("\033[1;34m[INFO]\033[0m Client %s searched for: %s\n", client_ip, keyword);
        } else if (strncmp(buffer, "upload ", 7) == 0) {
            char *path = buffer + 7;
            path[strcspn(path, "\n")] = 0;
            upload_site(client_socket, path);
            printf("\033[1;34m[INFO]\033[0m Client %s uploaded site: %s\n", client_ip, path);
        } else if (strncmp(buffer, "help", 4) == 0) {
            show_help(client_socket);
            printf("\033[1;34m[INFO]\033[0m Client %s requested help\n", client_ip);
        } else if (strncmp(buffer, "quit", 4) == 0) {
            dprintf(client_socket, "\033[1;33m[INFO]\033[0m Goodbye!\n");
            printf("\033[1;33m[INFO]\033[0m Client %s disconnected\n", client_ip);
            break;
        } else {
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Unknown command. Type 'help' for available commands.\n");
            printf("\033[1;31m[ERROR]\033[0m Client %s entered unknown command: %s\n", client_ip, buffer);
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    close(client_socket);
}

void open_spp_file(int client_socket, const char* url) {
    char site_name[256], file_name[256];
    if (parse_spp_url(url, site_name, file_name)) {
        char file_path[512];
        if (get_file_path(site_name, file_name, file_path)) {
            SPPFile *file = parse_spp_file(file_path);
            if (file) {
                render_spp_file(client_socket, file, site_name);
                free_spp_file(file);
            } else {
                dprintf(client_socket, "\033[1;31m[ERROR]\033[0m File not found or invalid\n");
            }
        } else {
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Site or file not found\n");
        }
    } else {
        dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Invalid SPP URL\n");
    }
}

void list_spp_sites(int client_socket) {
    SiteInfo *sites;
    int site_count = get_all_sites(&sites);

    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Available SPP sites:\n");
    for (int i = 0; i < site_count; i++) {
        dprintf(client_socket, "  superpuper://%s-%s\n", sites[i].name, sites[i].salt);
    }

    free(sites);
}

void search_files(int client_socket, const char* keyword) {
    SiteInfo *sites;
    int site_count = get_all_sites(&sites);

    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Search results for '%s':\n", keyword);
    int results = 0;

    for (int i = 0; i < site_count; i++) {
        char site_path[512];
        snprintf(site_path, sizeof(site_path), "sites/%s-%s", sites[i].name, sites[i].salt);

        DIR *dir = opendir(site_path);
        if (dir == NULL) continue;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".spp") != NULL) {
                char file_path[768];
                snprintf(file_path, sizeof(file_path), "%s/%s", site_path, entry->d_name);

                FILE *file = fopen(file_path, "r");
                if (file == NULL) continue;

                char line[1024];
                while (fgets(line, sizeof(line), file) != NULL) {
                    if (strcasestr(line, keyword) != NULL) {
                        dprintf(client_socket, "  superpuper://%s-%s/%s\n", sites[i].name, sites[i].salt, entry->d_name);
                        results++;
                        break;
                    }
                }

                fclose(file);
            }
        }

        closedir(dir);
    }

    if (results == 0) {
        dprintf(client_socket, "  No results found.\n");
    }

    free(sites);
}

void upload_site(int client_socket, const char* path) {
    char site_name[256], salt[16];
    generate_salt(salt);

    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        strcpy(site_name, last_slash + 1);
    } else {
        strcpy(site_name, path);
    }

    char site_path[512];
    snprintf(site_path, sizeof(site_path), "sites/%s-%s", site_name, salt);

    if (mkdir(site_path, 0755) == 0) {
        char command[1024];
        snprintf(command, sizeof(command), "cp -R %s/* %s/", path, site_path);
        if (system(command) == 0) {
            add_site(site_name, salt);
            dprintf(client_socket, "\033[1;32m[INFO]\033[0m Site uploaded successfully. URL: superpuper://%s-%s\n", site_name, salt);
        } else {
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Failed to copy site files\n");
            rmdir(site_path);
        }
    } else {
        dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Failed to create site directory\n");
    }
}

void show_help(int client_socket) {
    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Available commands:\n");
    dprintf(client_socket, "  open <superpuper://site-salt/file.spp> - Open and display an SPP file\n");
    dprintf(client_socket, "  list - List all available SPP sites\n");
    dprintf(client_socket, "  stats - Show connection statistics\n");
    dprintf(client_socket, "  search <keyword> - Search for a keyword in all SPP files\n");
    dprintf(client_socket, "  upload <path> - Upload a new site\n");
    dprintf(client_socket, "  help - Show this help message\n");
    dprintf(client_socket, "  quit - Disconnect from the server\n");
}

void show_stats(int client_socket, time_t connect_time) {
    time_t current_time = time(NULL);
    double elapsed = difftime(current_time, connect_time);

    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Connection Statistics:\n");
    dprintf(client_socket, "  Connected for: %.0f seconds\n", elapsed);
    dprintf(client_socket, "  Current time: %s", ctime(&current_time));
}

void get_client_ip(int client_socket, char *ip) {
    struct sockaddr_storage addr;
    socklen_t addr_size = sizeof(struct sockaddr_storage);
    getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);
    inet_ntop(AF_INET, &((struct sockaddr_in*)&addr)->sin_addr, ip, 16);
}