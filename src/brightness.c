#include "util.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

int get_brightness();

void set_brightness(const int percent);

//

void handle_brightness_change(GtkRange *scale, gpointer data) {

    GtkLabel *label = GTK_LABEL(data);

    double value = gtk_range_get_value(scale);
    char *str = g_strdup_printf("%3.0f", value);

    gtk_label_set_label(label, str);
    set_brightness((int)value);

    g_free(str);
}

void handle_brightness(GtkBuilder *builder) {
    GtkLabel *label =
        GTK_LABEL(gtk_builder_get_object(builder, "brightness-scale-label"));
    GtkScale *scale =
        GTK_SCALE(gtk_builder_get_object(builder, "brightness-scale"));

    g_signal_connect(scale, "value-changed",
                     G_CALLBACK(handle_brightness_change), label);
    // Set initial value, also calls handle_brightness_change()
    gtk_range_set_value(GTK_RANGE(scale), get_brightness());
}

//

int get_brightness() {
    char brightness[4000];
    exec_command(brightness, sizeof(brightness), "brightnessctl get", "r");
    char max_brightness[4000];
    exec_command(max_brightness, sizeof(brightness), "brightnessctl max", "r");

    double percentage = (atof(brightness) / atof(max_brightness) * 100);

    return (int)percentage;
}

void set_brightness(const int percent) {
    char *command = malloc(PATH_MAX);
    snprintf(command, PATH_MAX, "brightnessctl set %i%%", percent);

    char res[4000];
    exec_command(res, sizeof(res), command, "r");

    free(command);
}
