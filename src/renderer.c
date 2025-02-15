#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "renderer.h"

void render_spp_file(int client_socket, const SPPFile* file, const char* site_name) {
    dprintf(client_socket, "\033[2J\033[H");  // Clear screen and move cursor to top-left
    dprintf(client_socket, "\033[1;33m%s\033[0m\n\n", file->title);

    for (int i = 0; i < file->section_count; i++) {
        const SPPSection* section = &file->sections[i];
        dprintf(client_socket, "\033[1;33m%s\033[0m\n", section->title);
        
        if (section->content) {
            dprintf(client_socket, "%s\n", section->content);
        }

        for (int j = 0; j < section->link_count; j++) {
            const SPPLink* link = &section->links[j];
            if (link->number == 0) {
                // Internal page link
                dprintf(client_socket, "  \033[34m@page %s\033[0m\n", link->text);
            } else {
                // External link
                dprintf(client_socket, "  \033[34m[%d] %s - %s\033[0m\n", link->number, link->text, link->url);
            }
        }

        dprintf(client_socket, "\n");
    }

    dprintf(client_socket, "\nTo open a linked page, use: open superpuper://%s/<page_name>.spp\n", site_name);
}