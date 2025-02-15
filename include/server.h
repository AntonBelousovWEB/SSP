#ifndef SERVER_H
#define SERVER_H

#include <time.h>

void handle_client(int client_socket);
void open_spp_file(int client_socket, const char* url);
void list_spp_sites(int client_socket);
void show_stats(int client_socket, time_t connect_time);
void search_files(int client_socket, const char* keyword);
void upload_site(int client_socket, const char* path);
void show_help(int client_socket);
void get_client_ip(int client_socket, char *ip);

#endif