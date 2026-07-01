#include "types.h"
#include "util.h"
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>

static void dbus_handle_boost_return(GObject *obj, GAsyncResult *res, gpointer data) {
    GtkButton *button = GTK_BUTTON(data);
    GDBusConnection *conn = G_DBUS_CONNECTION(obj);
    GError *error = NULL;

    GVariant *result = g_dbus_connection_call_finish(conn, res, &error);

    if (!result) {
        g_warning("DBus boost return failed, %s", error->message);
        g_error_free(error);
        return;
    }

    gint32 is_boost;
    g_variant_get(result, "(b)", &is_boost);

    set_button_active_class(is_boost, button);

    g_variant_unref(result);
}

static void boost_mode_click_handler(gpointer *data) {
    GtkButton *button = GTK_BUTTON(data);

    State *state = (State *)data;

    g_dbus_connection_call(
        state->connection,
        "com.vinii.SysUiDaemon",
        "/com/vinii/BoostController",
        "com.vinii.BoostController",
        "SetBoost", // !current
        NULL,
        G_VARIANT_TYPE("(b)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        dbus_handle_boost_return,
        button
    );
}

static gboolean boost_mode_timeout_handler(void *data) {
    GtkButton *button = GTK_BUTTON(data);

    State *state = (State *)data;

    g_dbus_connection_call(
        state->connection,
        "com.vinii.SysUiDaemon",
        "/com/vinii/BoostController",
        "com.vinii.BoostController",
        "GetBoost",
        NULL,
        G_VARIANT_TYPE("(b)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        dbus_handle_boost_return,
        button
    );

    return TRUE;
}

void handle_boost_mode(GtkBuilder *builder, State *state) {
    GtkButton *boost_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "boost-mode"));

    if (G_UNLIKELY(state->connection == NULL)) {
        g_error("DBus connection unavailable");
        return;
    }

    g_signal_connect(
        boost_button, "clicked", G_CALLBACK(boost_mode_click_handler), state
    );

    boost_mode_timeout_handler(boost_button);
    g_timeout_add_seconds(1, boost_mode_timeout_handler, boost_button);
}
