#include "util.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

// TODO:
typedef enum {
    BRIGHTNESS_PATH,
} Scale;

int get_brightness() {
    char *max_brightness =
        read_file("/sys/class/backlight/amdgpu_bl1/max_brightness");
    char *brightness = read_file("/sys/class/backlight/amdgpu_bl1/brightness");

    int percent = atoi(brightness) / atoi(max_brightness) * 100;

    return percent;
}

void set_brightness(const int percent) {
    char *str = malloc(sizeof(char) * 50);
    sprintf(str, "%i", percent);

    write_file("/sys/class/backlight/amdgpu_bl1/brightness", str);
    free(str);
}

void handle_brightness_change(GtkRange *scale, gpointer data) {
    char *max_brightness =
        read_file("/sys/class/backlight/amdgpu_bl1/max_brightness");

    GtkLabel *label = GTK_LABEL(data);

    double value = gtk_range_get_value(scale);
    char *str = g_strdup_printf("%3.0f", value);

    gtk_label_set_label(label, str);
    set_brightness(value / 100 * atoi(max_brightness));

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
