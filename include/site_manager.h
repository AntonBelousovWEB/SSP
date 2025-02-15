#ifndef SITE_MANAGER_H
#define SITE_MANAGER_H

#include <stddef.h>

typedef struct {
    char name[256];
    char salt[16];
} SiteInfo;

int parse_spp_url(const char* url, char* site_name, char* file_name);
int get_file_path(const char* site_name, const char* file_name, char* file_path);
void generate_salt(char* salt);
int get_all_sites(SiteInfo **sites);
void add_site(const char* name, const char* salt);

#endif // SITE_MANAGER_H