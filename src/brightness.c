#include "types.h"
#include <gtk/gtk.h>

typedef struct {
    GDBusConnection *connection;
    GtkLabel *label;
} SetterData;

static void handle_brightness_change(GtkRange *scale, gpointer data) {
    SetterData *setter_data = (SetterData *)data;

    double value = gtk_range_get_value(scale);
    char *str = g_strdup_printf("%3.0f", value);

    gtk_label_set_label(setter_data->label, str);

    g_dbus_connection_call(
        setter_data->connection,
        "com.vinii.SysUiDaemon",
        "/com/vinii/BrightnessController",
        "com.vinii.BrightnessController",
        "SetBrightness",
        g_variant_new("(i)", (gint)value),
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        NULL,
        NULL
    );

    g_free(str);
}

static void dbus_get_brightness(
    GObject *obj, GAsyncResult *res, gpointer data
) {
    GDBusConnection *conn = G_DBUS_CONNECTION(obj);
    GError *error = NULL;
    GtkRange *scale = GTK_RANGE(data);

    GVariant *result = g_dbus_connection_call_finish(conn, res, &error);

    if (!result) {
        g_warning("DBus get brightness failed, %s", error->message);
        g_error_free(error);
        return;
    }

    gint32 brightness;
    g_variant_get(result, "(i)", &brightness);

    gtk_range_set_value(scale, brightness);

    g_variant_unref(result);
}

void handle_brightness(GtkBuilder *builder, State *state) {
    GtkLabel *label =
        GTK_LABEL(gtk_builder_get_object(builder, "brightness-scale-label"));
    GtkScale *scale =
        GTK_SCALE(gtk_builder_get_object(builder, "brightness-scale"));

    if (G_UNLIKELY(state->connection == NULL)) {
        g_error("DBus connection unavailable");
        return;
    }

    SetterData *setter_data = g_new0(SetterData, 1);
    setter_data->connection = state->connection;
    setter_data->label = label;

    g_signal_connect(
        scale,
        "value-changed",
        G_CALLBACK(handle_brightness_change),
        setter_data
    );

    g_object_set_data_full(
        G_OBJECT(scale), "setter-data-cleanup", setter_data, g_free
    );

    g_dbus_connection_call(
        state->connection,
        "com.vinii.SysUiDaemon",
        "/com/vinii/BrightnessController",
        "com.vinii.BrightnessController",
        "GetBrightness",
        NULL,
        G_VARIANT_TYPE("(i)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        dbus_get_brightness,
        scale
    );
}
