#include "path_utils.h"
#include "config.h"

// Function implementations
// Function to get paths from $PATH
char** get_path_dirs(int *count) {
    char *path_env = getenv("PATH");
    char *path = strdup(path_env);
    char *token;
    char **dirs = NULL;
    *count = 0;

    token = strtok(path, ":");
    while (token != NULL) {
        dirs = realloc(dirs, sizeof(char*) * (++(*count)));
        dirs[*count - 1] = strdup(token);
        token = strtok(NULL, ":");
    }
    free(path);
    return dirs;
}

// Check if file is an executable
int is_executable(const char *filepath) {
    return access(filepath, X_OK) == 0;
}

int is_full_match(const char *input, ResultList *result_list) {
    for (int i = 0; i < result_list->count; i++) {
        if (strcmp(input, result_list->items[i]) == 0) {
            return 1;  // Found a full match
        }
    }
    return 0;  // No full match found
}

// Search for binary in directories
int search_binaries(const char *query, ResultList *result_list) {
    int dir_count = 0;
    char **dirs = get_path_dirs(&dir_count);
    struct dirent *entry;
    char filepath[1024];

    result_list->count = 0;
    result_list->selected = 0;

    for (int i = 0; i < dir_count; i++) {
        DIR *dir = opendir(dirs[i]);
        if (dir) {
            while ((entry = readdir(dir)) != NULL) {
                if (strncmp(entry->d_name, query, strlen(query)) == 0) {
                    snprintf(filepath, sizeof(filepath), "%s/%s", dirs[i], entry->d_name);
                    if (is_executable(filepath)) {
                        int is_duplicate = 0;
                        for (int j = 0; j < result_list->count; j++) {
                            if (strcmp(result_list->items[j], entry->d_name) == 0) {
                                is_duplicate = 1;
                                break;
                            }
                        }
                        if (!is_duplicate && result_list->count < MAX_RESULTS) {
                            result_list->items[result_list->count++] = strdup(entry->d_name);
                        }
                    }
                }
            }
            closedir(dir);
        }
    }

    for (int i = 0; i < dir_count; i++) {
        free(dirs[i]);
    }
    free(dirs);
    return result_list->count;
}