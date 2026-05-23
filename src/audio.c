#include "util.h"
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_volume_percent() {
    char output[4000];
    exec_command(output, sizeof(output),
                 "wpctl get-volume @DEFAULT_SINK@ | awk '{print $2}'", "r");

    float res = atof(output) * 100.0;

    return res;
}

void set_volume(const int percent) {
    int value = percent;
    if (value > 100) {
        value = 100;
    } else if (value < 0) {
        value = 0;
    }

    char command[PATH_MAX];
    snprintf(command, PATH_MAX, "wpctl set-volume @DEFAULT_SINK@ %i%%", value);

    char res[4000];
    exec_command(res, sizeof(res), command, "r");
}

bool get_is_muted() {
    char output[4000];
    exec_command(output, sizeof(output),
                 "wpctl get-volume @DEFAULT_SINK@ | awk '{print $3}'", "r");

    if (strcmp(output, "[MUTED]") == 0) {
        return true;
    }

    return false;
}

//

void handle_audio_change(GtkRange *scale, gpointer data) {

    GtkLabel *label = GTK_LABEL(data);

    double value = gtk_range_get_value(scale);
    char *str = g_strdup_printf("%3.0f", value);

    gtk_label_set_label(label, str);
    set_volume((int)(value));

    g_free(str);
}

void handle_audio(GtkBuilder *builder) {
    GtkLabel *label =
        GTK_LABEL(gtk_builder_get_object(builder, "volume-scale-label"));
    GtkScale *scale =
        GTK_SCALE(gtk_builder_get_object(builder, "volume-scale"));

    g_signal_connect(scale, "value-changed", G_CALLBACK(handle_audio_change),
                     label);
    // Set initial value, also calls handle_audio_change()
    gtk_range_set_value(GTK_RANGE(scale), get_volume_percent());
}
