#include "util.h"
#include <dirent.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *path = "/sys/class/backlight";
static char *max = "/max_brightness";
static char *brightness = "/brightness";

int get_brightness();

void set_brightness(const int percent, const char *filepath);

void set_brightness_all(const int percent);

//

void handle_brightness_change(GtkRange *scale, gpointer data) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("Failed to open directory");
        exit(1);
    }

    char *max_brightness = malloc(sizeof(char) * 50);
    if (max_brightness == NULL) {
        perror("Failed to alloc");
        exit(1);
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            perror("Failed to alloc");
            exit(1);
        }
        snprintf(filepath, PATH_MAX, "%s/%s%s", path, entry->d_name, max);

        snprintf(max_brightness, sizeof(strlen(filepath)), "%s",
                 read_file(filepath));

        break;
    }

    GtkLabel *label = GTK_LABEL(data);

    double value = gtk_range_get_value(scale);
    char *str = g_strdup_printf("%3.0f", value);

    gtk_label_set_label(label, str);
    set_brightness_all(value / 100 * atoi(max_brightness));

    free(max_brightness);
    g_free(str);
}

void handle_brightness(GtkBuilder *builder) {
    GtkLabel *label =
        GTK_LABEL(gtk_builder_get_object(builder, "brightness-scale-label"));
    GtkScale *scale =
        GTK_SCALE(gtk_builder_get_object(builder, "brightness-scale"));

    g_signal_connect(scale, "value-changed",
                     G_CALLBACK(handle_brightness_change), label);

    gtk_range_set_value(GTK_RANGE(scale), get_brightness());
}

//

int get_brightness() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("Failed to open directory");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            perror("Failed to alloc");
            exit(1);
        }

        // Operate on the first file it finds
        snprintf(filepath, PATH_MAX, "%s/%s%s", path, entry->d_name, max);
        char *max_brightness = read_file(filepath);
        free(filepath);

        filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            perror("Failed to alloc");
            exit(1);
        }
        snprintf(filepath, PATH_MAX, "%s/%s%s", path, entry->d_name, max);
        char *brightness = read_file(filepath);

        int percent = atoi(brightness) / atoi(max_brightness) * 100;

        free(filepath);
        return percent;
    }

    return 0;
}

void set_brightness(const int percent, const char *filepath) {
    char *str = malloc(sizeof(char) * 50);
    sprintf(str, "%i", percent);

    write_file(filepath, str);
    free(str);
}

void set_brightness_all(const int percent) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("Failed to open directory");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            perror("Failed to alloc");
            exit(1);
        }

        snprintf(filepath, PATH_MAX, "%s/%s%s", path, entry->d_name,
                 brightness);

        set_brightness(percent, filepath);
        free(filepath);
    }

    closedir(dir);
}
