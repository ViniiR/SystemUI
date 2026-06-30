#include <gtk/gtk.h>
#include <stdbool.h>

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
