#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

ResultString read_file(const char *path) {
    FILE *f = fopen(path, "r");
    ResultString res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = ""
    };

    if (f == NULL) {
        res.err_msg = "File is null";
        return res;
    }

    char *max = malloc(sizeof(char) * 50);
    if (max == NULL) {
        free(max);
        fclose(f);

        res.err_msg = "Failed to alloc";
        return res;
    }

    char *value = fgets(max, 50, f);
    if (value == NULL) {
        free(max);
        fclose(f);

        res.err_msg = "Failed to read file content";
        return res;
    }
    // Remove trailing newlines
    value[strcspn(value, "\r\n")] = 0;

    fclose(f);

    res.variant = OK;
    res.err_msg = "";
    res.ok_value = value;
    return res;
}

ResultVoid write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    ResultVoid res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = NULL
    };

    if (f == NULL) {
        res.err_msg = "Failed to open file";
        return res;
    }

    fprintf(f, "%s", content);

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
    static thread_local char error_message[4096];

    ResultVoid res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = NULL
    };
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
