#include "util.h"
#include <gtk/gtk.h>
#include <stdio.h>

void handle_reboot(GtkButton *button, gpointer data) {
    char res[4000];
    exec_command(res, sizeof(res), "systemctl reboot", "r");
}

void handle_shutdown(GtkButton *button, gpointer data) {
    char res[4000];
    exec_command(res, sizeof(res), "systemctl poweroff", "r");
}
void handle_logout() {
    char res[4000];
    perror("BRUV INNNIT");
    // TODO: might be DE/WM specific
    // exec_command(res, sizeof(res), "systemctl ", "r");
}

//

void handle_power_buttons(GtkBuilder *builder) {
    GtkButton *reboot_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "reboot-button"));
    GtkButton *shutdown_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "shutdown-button"));
    GtkButton *logout_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "logout-button"));

    g_signal_connect(reboot_button, "clicked", G_CALLBACK(handle_reboot), NULL);
    g_signal_connect(shutdown_button, "clicked", G_CALLBACK(handle_shutdown),
                     NULL);
}
