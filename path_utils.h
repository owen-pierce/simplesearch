#ifndef BINARIES_H
#define BINARIES_H

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include "config.h"

typedef struct {
    int count;
    int selected;
    char *items[MAX_RESULTS];
} ResultList;

char** get_path_dirs(int *count);
int is_executable(const char *filepath);
int search_binaries(const char *query, ResultList *result_list);
int is_full_match(const char *input, ResultList *result_list);

#endif
