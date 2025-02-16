#ifndef UTILS_H
#define UTILS_H

#include <time.h>

void get_client_ip(int client_socket, char *ip);
void show_stats(int client_socket, time_t connect_time);
void search_files(int client_socket, const char* keyword);
void show_help(int client_socket);

#endif