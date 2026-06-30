#pragma once

#include <gtk/gtk.h>

char *read_file(const char *path);

void write_file(const char *path, const char *content);

[[noreturn]] void safe_fail(const char *message);

void exec_command(char *output, const unsigned int size, const char *command,
                  const char *modes);

void set_button_active_class(const bool value, GtkButton *button);
