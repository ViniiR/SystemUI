#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: gracefully quit the app instead of exit(1)
[[noreturn]] void safe_fail(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void set_button_active_class(const bool value, GtkButton *button) {
    if (value) {
        gtk_widget_remove_css_class(GTK_WIDGET(button), "button-inactive");
        gtk_widget_add_css_class(GTK_WIDGET(button), "button-active");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(button), "button-active");
        gtk_widget_add_css_class(GTK_WIDGET(button), "button-inactive");
    }
}
