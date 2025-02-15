#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define MAX_LINE_LENGTH 1024

SPPFile* parse_spp_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        return NULL;
    }

    SPPFile* spp_file = malloc(sizeof(SPPFile));
    spp_file->title = NULL;
    spp_file->sections = NULL;
    spp_file->section_count = 0;

    char line[MAX_LINE_LENGTH];
    SPPSection* current_section = NULL;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline

        if (strncmp(line, "@title ", 7) == 0) {
            spp_file->title = strdup(line + 7);
        } else if (line[0] == '#') {
            spp_file->section_count++;
            spp_file->sections = realloc(spp_file->sections, spp_file->section_count * sizeof(SPPSection));
            current_section = &spp_file->sections[spp_file->section_count - 1];
            current_section->title = strdup(line + 2);
            current_section->content = NULL;
            current_section->links = NULL;
            current_section->link_count = 0;
        } else if (current_section) {
            if (line[0] == '[') {
                current_section->link_count++;
                current_section->links = realloc(current_section->links, current_section->link_count * sizeof(SPPLink));
                SPPLink* link = &current_section->links[current_section->link_count - 1];
                sscanf(line, "[%d] %[^-]- %s", &link->number, link->text, link->url);
            } else if (line[0] == '@' && strncmp(line, "@page ", 6) == 0) {
                current_section->link_count++;
                current_section->links = realloc(current_section->links, current_section->link_count * sizeof(SPPLink));
                SPPLink* link = &current_section->links[current_section->link_count - 1];
                link->number = 0;  // Use 0 to indicate an internal page link
                strcpy(link->text, line + 6);
                strcpy(link->url, line + 6);
            } else {
                size_t content_len = current_section->content ? strlen(current_section->content) : 0;
                size_t new_len = content_len + strlen(line) + 2;  // +2 for newline and null terminator
                current_section->content = realloc(current_section->content, new_len);
                if (content_len > 0) {
                    strcat(current_section->content, "\n");
                }
                strcat(current_section->content, line);
            }
        }
    }

    fclose(file);
    return spp_file;
}

void free_spp_file(SPPFile* file) {
    if (!file) return;

    free(file->title);
    for (int i = 0; i < file->section_count; i++) {
        free(file->sections[i].title);
        free(file->sections[i].content);
        free(file->sections[i].links);
    }
    free(file->sections);
    free(file);
}