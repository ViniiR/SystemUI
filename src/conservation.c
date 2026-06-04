#include "util.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char filepath[] =
    "/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode";

bool get_is_conservation_active() {
    char *res = read_file(filepath);
    bool is_active = false;
    if (strcmp(res, "1") == 0) {
        is_active = true;
    }

    free(res);
    return is_active;
}

void set_conservation_mode(const bool value) {
    char command[PATH_MAX];

    snprintf(command, sizeof(command), "echo %i | sudo tee %s > /dev/null",
             value, filepath);

    exec_command(NULL, 0, command, "r");
}

void set_active_class(const bool value, GtkButton *button) {
    if (value) {
        gtk_widget_remove_css_class(GTK_WIDGET(button), "button-inactive");
        gtk_widget_add_css_class(GTK_WIDGET(button), "button-active");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(button), "button-active");
        gtk_widget_add_css_class(GTK_WIDGET(button), "button-inactive");
    }
}

gboolean conservation_mode_timeout_handler(void *data) {
    GtkButton *button = GTK_BUTTON(data);

    set_active_class(get_is_conservation_active(), button);

    return TRUE;
}

void conservation_mode_click_handler(gpointer *data) {
    GtkButton *button = GTK_BUTTON(data);
    bool is_conservation_active = get_is_conservation_active();

    set_active_class(is_conservation_active, button);
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
