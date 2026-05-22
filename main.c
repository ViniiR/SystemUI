#include "handlers.h"
#include <gtk/gtk.h>
#include <locale.h>

static void activate(GApplication *app, gpointer *user_data) {
    // THAT'S THE MOST ABSURD THING I'VE EVER SEEN AAHHHH
    setlocale(LC_NUMERIC, "C");
    GtkBuilder *builder = gtk_builder_new_from_file("../src/builder.ui");

    GtkWidget *win = GTK_WIDGET(gtk_builder_get_object(builder, "main-window"));

    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));
    gtk_window_present(GTK_WINDOW(win));

    handle_brightness(builder);
    handle_audio(builder);
    handle_power_buttons(builder);
    handle_battery(builder);
}

// TODO: gracefully quit the app instead of exit(1)
int main(int argc, char **argv) {
    GtkApplication *app;
    int status = 0;

    app =
        gtk_application_new("com.vinii.system_ui", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
