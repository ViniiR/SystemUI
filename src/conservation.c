#include "util.h"
#include <gtk/gtk.h>
#include <stdio.h>

gboolean conservation_mode_timeout_handler(void *data) {
    GtkButton *button = GTK_BUTTON(data);

    set_button_active_class(get_is_conservation_active(), button);

    return TRUE;
}

void conservation_mode_click_handler(gpointer *data) {
    GtkButton *button = GTK_BUTTON(data);
    bool is_conservation_active = get_is_conservation_active();

    set_button_active_class(is_conservation_active, button);
    set_conservation_mode(!is_conservation_active);
}

void handle_conservation_mode(GtkBuilder *builder) {
    GtkButton *conservation_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "conservation-mode-button"));

    g_signal_connect(conservation_button, "clicked",
                     G_CALLBACK(conservation_mode_click_handler), NULL);

    conservation_mode_timeout_handler(conservation_button);
    g_timeout_add_seconds(1, conservation_mode_timeout_handler,
                          conservation_button);
}
