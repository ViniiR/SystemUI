#pragma once

char *read_file(const char *path);

void write_file(const char *path, const char *content);

void safe_fail(const char *message);

char *exec_command(const char *command, const char *modes);
