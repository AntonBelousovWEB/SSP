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
#define MAX_CONTEXT_LENGTH 255

char current_context[MAX_CONTEXT_LENGTH + 1] = {0};

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
            memset(current_context, 0, sizeof(current_context));
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
        } else if (current_context[0] != '\0' && strchr(buffer, '/') == NULL) {
            char full_url[BUFFER_SIZE];
            snprintf(full_url, sizeof(full_url), "%s/%s", current_context, buffer);
            full_url[BUFFER_SIZE - 1] = '\0';
            open_spp_file(client_socket, full_url);
            printf("\033[1;34m[INFO]\033[0m Client %s opened file: %s\n", client_ip, full_url);
        } else {
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Unknown command. Type 'help' for available commands.\n");
            printf("\033[1;31m[ERROR]\033[0m Client %s entered unknown command: %s\n", client_ip, buffer);
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    close(client_socket);
}

void open_spp_file(int client_socket, const char* url) {
    memset(current_context, 0, sizeof(current_context));
    char site_name[256], file_name[256];
    if (parse_spp_url(url, site_name, file_name)) {
        char file_path[512];
        if (get_file_path(site_name, file_name, file_path)) {
            printf("\033[1;34m[DEBUG]\033[0m Attempting to open file: %s\n", file_path);
            SPPFile *file = parse_spp_file(file_path);
            if (file) {
                render_spp_file(client_socket, file, site_name);
                free_spp_file(file);
                snprintf(current_context, sizeof(current_context), "superpuper://%s", site_name);
                current_context[MAX_CONTEXT_LENGTH] = '\0';  // Ensure null-termination
            } else {
                dprintf(client_socket, "\033[1;31m[ERROR]\033[0m File not found or invalid: %s\n", file_path);
                printf("\033[1;31m[ERROR]\033[0m Failed to parse file: %s\n", file_path);
            }
        } else {
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Site or file not found: %s\n", url);
            printf("\033[1;31m[ERROR]\033[0m Failed to get file path for: %s\n", url);
        }
    } else {
        dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Invalid SPP URL: %s\n", url);
        printf("\033[1;31m[ERROR]\033[0m Invalid SPP URL: %s\n", url);
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

void upload_site(int client_socket, const char* path) {
    char site_name[256], salt[16];
    generate_salt(salt);

    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        strcpy(site_name, last_slash + 1);
    } else {
        strcpy(site_name, path);
    }

    size_t len = strlen(site_name);
    if (len > 0 && site_name[len - 1] == '/') {
        site_name[len - 1] = '\0';
    }

    char site_path[512];
    snprintf(site_path, sizeof(site_path), "sites/%s-%s", site_name, salt);

    if (mkdir(site_path, 0755) == 0) {
        char command[1024];
        snprintf(command, sizeof(command), "cp -R %s/* %s/ 2>&1", path, site_path);
        FILE *pipe = popen(command, "r");
        if (pipe == NULL) {
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Failed to execute copy command\n");
            rmdir(site_path);
            return;
        }

        char buffer[1024];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), pipe);
        int status = pclose(pipe);

        if (status == 0 && bytes_read == 0) {
            add_site(site_name, salt);
            dprintf(client_socket, "\033[1;32m[INFO]\033[0m Site uploaded successfully. URL: superpuper://%s-%s\n", site_name, salt);
        } else {
            buffer[bytes_read] = '\0';
            dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Failed to copy site files. Error: %s\n", buffer);
            rmdir(site_path);
        }
    } else {
        dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Failed to create site directory\n");
    }
}