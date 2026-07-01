#include "handlers.h"
#include "types.h"
#include <gtk/gtk.h>
#include <locale.h>
#include <stdlib.h>

static void activate(GApplication *app, gpointer *data) {
    // THAT'S THE MOST ABSURD THING I'VE EVER SEEN AAHHHH
    setlocale(LC_NUMERIC, "C");

    GtkBuilder *builder = gtk_builder_new_from_file("../src/ui/builder.ui");
    GtkWidget *win = GTK_WIDGET(gtk_builder_get_object(builder, "main-window"));
    GtkCssProvider *css_provider = gtk_css_provider_new();

    State *state = (State *)data;

    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));
    gtk_window_present(GTK_WINDOW(win));

    handle_brightness(builder, state);
    // handle_audio(builder);
    handle_power_buttons(builder, state);
    handle_battery(builder);
    handle_conservation_mode(builder);
    handle_boost_mode(builder);

    gtk_css_provider_load_from_path(css_provider, "../src/style.css");

    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status = 0;

    State *program_state = g_new0(State, 1);
    GError *error = NULL;

    program_state->connection =
        g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (!program_state->connection) {
        g_warning("Failed to connect to DBus, Message: %s", error->message);
        g_error_free(error);
        return EXIT_FAILURE;
    }

    app =
        gtk_application_new("com.vinii.system_ui", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), program_state);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    g_object_unref(program_state->connection);
    g_free(error);

    return status;
}
