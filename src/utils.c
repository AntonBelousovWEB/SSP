#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "utils.h"

void get_client_ip(int client_socket, char *ip) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);
    strcpy(ip, inet_ntoa(addr.sin_addr));
}

void show_stats(int client_socket, time_t connect_time) {
    time_t current_time = time(NULL);
    double elapsed = difftime(current_time, connect_time);
    
    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Connection Statistics:\n");
    dprintf(client_socket, "  Connected for: %.0f seconds\n", elapsed);
    // Add more stats here as needed
}

void search_files(int client_socket, const char* keyword) {
    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Searching for files containing '%s'...\n", keyword);
    dprintf(client_socket, "Search functionality not yet implemented.\n");
}

void show_help(int client_socket) {
    dprintf(client_socket, "\033[1;32m[INFO]\033[0m Available commands:\n");
    dprintf(client_socket, "  open <url> - Open an SPP file\n");
    dprintf(client_socket, "  list - List available SPP sites\n");
    dprintf(client_socket, "  stats - Show connection statistics\n");
    dprintf(client_socket, "  search <keyword> - Search for files (not yet implemented)\n");
    dprintf(client_socket, "  upload <path> - Upload a new site\n");
    dprintf(client_socket, "  help - Show this help message\n");
    dprintf(client_socket, "  quit - Disconnect from the server\n");
}