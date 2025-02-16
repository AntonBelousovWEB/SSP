#ifndef PARSER_H
#define PARSER_H

typedef struct {
    int number;
    char text[256];
    char url[256];
} SPPLink;

typedef struct {
    char* title;
    char* content;
    size_t content_size;
    SPPLink* links;
    int link_count;
} SPPSection;

typedef struct {
    char* title;
    SPPSection* sections;
    int section_count;
} SPPFile;

SPPFile* parse_spp_file(const char* filename);
void free_spp_file(SPPFile* file);

#endif