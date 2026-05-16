#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "File is null");
        exit(1);
    }

    char *max = malloc(sizeof(char) * 50);
    if (max == NULL) {
        fprintf(stderr, "File buffer is null");
        free(max);
        fclose(f);
        exit(1);
    }

    char *value = fgets(max, 50, f);
    if (value == NULL) {
        fprintf(stderr, "Failed to read file content");
        free(max);
        fclose(f);
        exit(1);
    }

    fclose(f);
    return value;
}

void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        exit(1);
    }

    fprintf(f, "%s", content);

    fclose(f);
}
