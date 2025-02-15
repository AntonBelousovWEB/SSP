#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "auth.h"

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32

int authenticate_client(int client_socket) {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    dprintf(client_socket, "Username: ");
    if (read(client_socket, username, sizeof(username)) <= 0) {
        return 0;
    }
    username[strcspn(username, "\n")] = 0;

    dprintf(client_socket, "Password: ");
    if (read(client_socket, password, sizeof(password)) <= 0) {
        return 0;
    }
    password[strcspn(password, "\n")] = 0;

    if (strcmp(username, "admin") == 0 && strcmp(password, "password") == 0) {
        dprintf(client_socket, "\033[1;32m[INFO]\033[0m Authentication successful\n");
        return 1;
    } else {
        dprintf(client_socket, "\033[1;31m[ERROR]\033[0m Authentication failed\n");
        return 0;
    }
}