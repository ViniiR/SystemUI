#pragma once

char *read_file(const char *path);

void write_file(const char *path, const char *content);

[[noreturn]] void safe_fail(const char *message);

void exec_command(char *output, const unsigned int size, const char *command,
                  const char *modes);
