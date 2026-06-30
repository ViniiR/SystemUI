#include "util.h"
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>

void boost_mode_click_handler(gpointer *data) {
    GtkButton *button = GTK_BUTTON(data);
    bool is_boost = get_is_boost_active();

    set_button_active_class(is_boost, button);
    set_boost_mode(!is_boost);
}

gboolean boost_mode_timeout_handler(void *data) {
    GtkButton *button = GTK_BUTTON(data);

    set_button_active_class(get_is_boost_active(), button);

    return TRUE;
}

void handle_boost_mode(GtkBuilder *builder) {
    GtkButton *boost_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "boost-mode"));

    g_signal_connect(boost_button, "clicked",
                     G_CALLBACK(boost_mode_click_handler), NULL);

    boost_mode_timeout_handler(boost_button);
    g_timeout_add_seconds(1, boost_mode_timeout_handler, boost_button);
}
