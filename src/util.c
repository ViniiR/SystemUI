#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

void safe_fail(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        safe_fail("File is null");
    }

    char *max = malloc(sizeof(char) * 50);
    if (max == NULL) {
        free(max);
        fclose(f);
        safe_fail("File buffer is null");
    }

    char *value = fgets(max, 50, f);
    if (value == NULL) {
        free(max);
        fclose(f);
        safe_fail("Failed to read file content");
    }

    fclose(f);
    return value;
}

void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        safe_fail("Failed to open file");
    }

    fprintf(f, "%s", content);

    fclose(f);
}

char *exec_command(const char *command, const char *modes) {
    FILE *fp;
    char *path = malloc(PATH_MAX);

    fp = popen(command, modes);
    if (fp == NULL) {
        safe_fail("Failed to run command");
    }

    // TODO: what if while never runs
    while (fgets(path, PATH_MAX, fp) != NULL) {
        // Only read first line
        break;
    }

    pclose(fp);
    return path;
}
