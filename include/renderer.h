#ifndef RENDERER_H
#define RENDERER_H

#include "parser.h"

void render_spp_file(int client_socket, const SPPFile* file, const char* site_name);

#endif