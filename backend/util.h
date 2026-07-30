#pragma once

#include "types.h"

ResultHeapString read_file(const char *path);

ResultVoid write_file(const char *path, const char *content);

ResultVoid exec_command(char *output, const unsigned int size, const char *command,
                  const char *modes);
