#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: gracefully quit the app instead of exit(1)
[[noreturn]] void safe_fail(const char *message) {
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
    // Remove trailing newlines
    value[strcspn(value, "\r\n")] = 0;

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

void exec_command(
    char *output,
    const unsigned int size,
    const char *command,
    const char *modes
) {
    FILE *fp;

    fp = popen(command, modes);
    if (fp == NULL) {
        char error_message[PATH_MAX];
        snprintf(
            error_message,
            sizeof(error_message),
            "Failed to execute Command: '%s'",
            command
        );
        safe_fail(error_message);
    }

    // ignore
    if (output != NULL && fgets(output, size, fp) == NULL) {
        pclose(fp);
        char error_message[PATH_MAX];
        snprintf(
            error_message,
            sizeof(error_message),
            "Could not read file, Command: '%s'",
            command
        );
        safe_fail(error_message);
    }

    pclose(fp);
}
