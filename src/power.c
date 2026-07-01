#include "types.h"
#include <gtk/gtk.h>
#include <stdio.h>

static void handle_reboot(gpointer *data) {
    State *state = (State *)data;

    g_dbus_connection_call(
        state->connection,
        "com.vinii.SysUiDaemon",
        "/com/vinii/PowerController",
        "com.vinii.PowerController",
        "Reboot",
        NULL,
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        NULL,
        NULL
    );
}

static void handle_shutdown(gpointer *data) {
    State *state = (State *)data;

    g_dbus_connection_call(
        state->connection,
        "com.vinii.SysUiDaemon",
        "/com/vinii/PowerController",
        "com.vinii.PowerController",
        "Shutdown",
        NULL,
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        NULL,
        NULL
    );
}

void handle_power_buttons(GtkBuilder *builder, State *state) {
    GtkButton *reboot_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "reboot-button"));
    GtkButton *shutdown_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "shutdown-button"));
    GtkButton *logout_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "logout-button"));

    if (G_UNLIKELY(state->connection == NULL)) {
        g_error("DBus connection unavailable");
        return;
    }

    g_signal_connect(reboot_button, "clicked", G_CALLBACK(handle_reboot), NULL);
    g_signal_connect(
        shutdown_button, "clicked", G_CALLBACK(handle_shutdown), NULL
    );
}
