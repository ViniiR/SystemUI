#include "types.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

ResultHeapString read_file(const char *path) {
    ResultHeapString res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = ""
    };

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        res.err_msg = "File is null";
        return res;
    }

    char *buffer = NULL;
    size_t len = 0;

    ssize_t bytes_read = getdelim(&buffer, &len, '\0', f); // malloc
    fclose(f);
    if (bytes_read == -1) {
        free(buffer);
        res.err_msg = "Failed to read file content";
        return res;
    }

    // Remove trailing newlines
    buffer[strcspn(buffer, "\r\n")] = 0;

    res.variant = OK;
    res.ok_value = buffer;
    res.err_msg = "";
    return res;
}

ResultVoid write_file(const char *path, const char *content) {
    ResultVoid res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = NULL
    };

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        res.err_msg = "Failed to open file";
        return res;
    }

    int chars_written = fprintf(f, "%s", content);
    if (chars_written == -1) {
        res.err_msg = "Failed to write to file";
        return res;
    }

    fclose(f);

    res.variant = OK;
    res.err_msg = "";
    return res;
}

ResultVoid exec_command(
    char *output,
    const unsigned int size,
    const char *command,
    const char *modes
) {
    static thread_local char error_message[STRING_KB];

    ResultVoid res = RESULT_VOID_DEFAULT;
    FILE *fp;

    fp = popen(command, modes);
    if (fp == NULL) {
        snprintf(
            error_message,
            sizeof(error_message),
            "Failed to execute Command: '%s'",
            command
        );
        res.err_msg = error_message;
        return res;
    }

    // ignore
    if (output != NULL && fgets(output, size, fp) == NULL) {
        pclose(fp);
        snprintf(
            error_message,
            sizeof(error_message),
            "Could not read file, Command: '%s'",
            command
        );
        res.err_msg = error_message;
        return res;
    }

    pclose(fp);

    res.variant = OK;
    res.err_msg = "";
    return res;
}
