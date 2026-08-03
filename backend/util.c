#include "sys/types.h"
#include "sys/wait.h"
#include "types.h"
#include "unistd.h"
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

ResultVoid exec_command_as_user(
    char *output,
    const unsigned int size,
    const char *command,
    const char *modes,
    const uid_t target_uid
) {
    ResultVoid res = RESULT_VOID_DEFAULT;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        res.err_msg = "Failed fork pipe";
        return res;
    }

    pid_t pid = fork();
    if (pid < 0) {
        res.err_msg = "Failed to fork process";
        return res;
    }

    // TODO: this is the only part of the application that doesn't propagate errors up.
    // Run as child process
    if (pid == 0) {
        close(pipefd[0]);

        char runtime_dir[64];
        snprintf(runtime_dir, sizeof(runtime_dir), "/run/user/%d", target_uid);
        setenv("XDG_RUNTIME_DIR", runtime_dir, 1);
        if (setgid(target_uid) != 0 || setuid(target_uid) != 0) {
            // res.err_msg = "Failed to drop process privileges";
            _exit(-1);
        }

        ResultVoid result_exec = exec_command(output, size, command, modes);
        if (result_exec.variant == ERR) {
            // res.err_msg = result_exec.err_msg;
            _exit(-2);
        }

        write(pipefd[1], output, size);
        close(pipefd[1]);

        _exit(0);
    }
    // Run as parent process
    else if (pid > 0) {
        close(pipefd[1]);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                res.err_msg = "Child process exited with failure code";
                printf("Terminated by code: %i\n", exit_code);
                return res;
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            res.err_msg = "Child process terminated by signal";
            printf("Terminated by signal: %i\n", sig);
            return res;
        } else {
            res.err_msg = "Process failed or exited abnormally";
            return res;
        }

        read(pipefd[0], output, size);
        close(pipefd[0]);

        res.variant = OK;
        res.err_msg = "";
        return res;
    }

    return res;
}
