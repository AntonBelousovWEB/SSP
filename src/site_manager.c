#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "site_manager.h"
#include <json-c/json.h>

#define SITES_FILE "sites.json"
#define MAX_SALT_LENGTH 16

int parse_spp_url(const char* url, char* site_name, char* file_name) {
    if (strncmp(url, "superpuper://", 13) != 0) return 0;

    const char* site_start = url + 13;
    const char* site_end = strchr(site_start, '/');
    
    if (site_end == NULL) {
        strcpy(site_name, site_start);
        file_name[0] = '\0';
    } else {
        size_t site_len = site_end - site_start;
        strncpy(site_name, site_start, site_len);
        site_name[site_len] = '\0';
        strcpy(file_name, site_end + 1);
    }

    printf("\033[1;34m[DEBUG]\033[0m Parsed URL - Site: %s, File: %s\n", site_name, file_name);
    return 1;
}

int get_file_path(const char* site_name, const char* file_name, char* file_path) {
    SiteInfo *sites;
    int site_count = get_all_sites(&sites);

    printf("\033[1;34m[DEBUG]\033[0m Searching for site: %s\n", site_name);

    for (int i = 0; i < site_count; i++) {
        char full_site_name[512];
        snprintf(full_site_name, sizeof(full_site_name), "%s-%s", sites[i].name, sites[i].salt);
        printf("\033[1;34m[DEBUG]\033[0m Comparing with: %s\n", full_site_name);
        
        if (strcmp(site_name, full_site_name) == 0) {
            if (file_name[0] == '\0') {
                snprintf(file_path, 512, "sites/%s/index.spp", full_site_name);
            } else {
                snprintf(file_path, 512, "sites/%s/%s", full_site_name, file_name);
            }
            printf("\033[1;34m[DEBUG]\033[0m File path constructed: %s\n", file_path);
            free(sites);
            return 1;
        }
    }

    printf("\033[1;31m[ERROR]\033[0m Site not found: %s\n", site_name);
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
    struct json_object *parsed_json;
    parsed_json = json_object_from_file(SITES_FILE);

    if (parsed_json == NULL) {
        *sites = NULL;
        return 0;
    }

    int site_count = json_object_array_length(parsed_json);
    *sites = malloc(sizeof(SiteInfo) * site_count);

    for (int i = 0; i < site_count; i++) {
        struct json_object *site_obj = json_object_array_get_idx(parsed_json, i);
        struct json_object *name_obj, *salt_obj;

        json_object_object_get_ex(site_obj, "name", &name_obj);
        json_object_object_get_ex(site_obj, "salt", &salt_obj);

        strncpy((*sites)[i].name, json_object_get_string(name_obj), sizeof((*sites)[i].name) - 1);
        strncpy((*sites)[i].salt, json_object_get_string(salt_obj), sizeof((*sites)[i].salt) - 1);
    }

    json_object_put(parsed_json);

    return site_count;
}


void add_site(const char* name, const char* salt) {
    SiteInfo *sites;
    int site_count = get_all_sites(&sites);

    struct json_object *root;
    if (site_count > 0) {
        root = json_object_from_file(SITES_FILE);
        if (root == NULL) {
            root = json_object_new_array();
        }
    } else {
        root = json_object_new_array();
    }

    struct json_object *new_site = json_object_new_object();
    json_object_object_add(new_site, "name", json_object_new_string(name));
    json_object_object_add(new_site, "salt", json_object_new_string(salt));
    json_object_array_add(root, new_site);

    json_object_to_file(SITES_FILE, root);

    json_object_put(root);
    free(sites);

    printf("\033[1;32m[DEBUG]\033[0m Site added: name=%s, salt=%s\n", name, salt);
}