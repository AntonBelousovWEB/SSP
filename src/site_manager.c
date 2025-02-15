#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "site_manager.h"

#define SITES_FILE "sites.json"
#define MAX_SALT_LENGTH 16

int parse_spp_url(const char* url, char* site_name, char* file_name) {
    if (strncmp(url, "superpuper://", 13) != 0) return 0;

    const char* site_start = url + 13;
    const char* site_end = strchr(site_start, '/');
    
    if (site_end == NULL) return 0;

    size_t site_len = site_end - site_start;
    strncpy(site_name, site_start, site_len);
    site_name[site_len] = '\0';

    strcpy(file_name, site_end + 1);

    return 1;
}

int get_file_path(const char* site_name, const char* file_name, char* file_path) {
    SiteInfo *sites;
    int site_count = get_all_sites(&sites);

    for (int i = 0; i < site_count; i++) {
        if (strcmp(site_name, sites[i].name) == 0) {
            snprintf(file_path, 512, "sites/%s-%s/%s", sites[i].name, sites[i].salt, file_name);
            free(sites);
            return 1;
        }
    }

    free(sites);
    return 0;
}

void generate_salt(char* salt) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_length = sizeof(charset) - 1;

    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_SALT_LENGTH - 1; i++) {
        salt[i] = charset[rand() % charset_length];
    }
    salt[MAX_SALT_LENGTH - 1] = '\0';
}

int get_all_sites(SiteInfo **sites) {
    FILE *file = fopen(SITES_FILE, "r");
    if (file == NULL) {
        *sites = NULL;
        return 0;
    }

    // TODO: Implement JSON parsing logic here
    // For now, we'll return a dummy site for demonstration
    *sites = malloc(sizeof(SiteInfo));
    strcpy((*sites)[0].name, "dummy_site");
    strcpy((*sites)[0].salt, "dummysalt");

    fclose(file);
    return 1;
}

void add_site(const char* name, const char* salt) {
    FILE *file = fopen(SITES_FILE, "a");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open sites file for writing\n");
        return;
    }

    fprintf(file, "{\"name\": \"%s\", \"salt\": \"%s\"}\n", name, salt);
    fclose(file);
}