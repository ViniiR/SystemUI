#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

static void get_battery_icon(char *output, const unsigned int size) {
    static const char plugged[] = "-plugged-in";
    static const char charging[] = "-charging";
    static const char battery_prefix[] = "battery-level-";

    int alloc_size = 1;
    char *suffix;
    switch (get_charging_status()) {
    case CHARGING:
        alloc_size += sizeof(charging);
        suffix = malloc(alloc_size);
        snprintf(suffix, alloc_size, "%s", charging);
        break;
    case PLUGGED_IN:
        alloc_size += sizeof(plugged);
        suffix = malloc(alloc_size);
        snprintf(suffix, alloc_size, "%s", plugged);
        break;
    case DISCHARGING:
        alloc_size += sizeof(char) * 2;
        suffix = malloc(alloc_size);
        break;
    }

    int battery_level_formatted = 0;
    int number_value = get_battery_percentage();

    if (number_value <= 0) {
        battery_level_formatted = 0;
    } else if (number_value <= 9) {
        battery_level_formatted = number_value;
    } else if (number_value >= 100) {
        battery_level_formatted = 100;
    } else {
        char first[1 + 1];
        snprintf(first, sizeof(first), "%1.0f", (float)number_value);
        battery_level_formatted = atoi(first) * 10;
    }

    char suffix_len = strlen(suffix);

    int inner_len = (sizeof(battery_prefix) + sizeof(battery_level_formatted) +
                     sizeof(char) * suffix_len);

    snprintf(output, size > inner_len ? size : inner_len, "%s%i%s",
             battery_prefix, battery_level_formatted, suffix);
    free(suffix);
}

static gboolean handle_battery_change(void *data) {
    GtkButton *button = GTK_BUTTON(data);
    GtkBox *box = GTK_BOX(gtk_button_get_child(button));

    char icon_name[4000];
    get_battery_icon(icon_name, sizeof(icon_name));

    // TODO: Hardcoded
    GtkWidget *label = gtk_widget_get_first_child(GTK_WIDGET(box));
    GtkWidget *image = gtk_widget_get_next_sibling(GTK_WIDGET(label));

    char text[10];

    snprintf(text, sizeof(text), "%.0f%%", (double)get_battery_percentage());

    gtk_label_set_label(GTK_LABEL(label), text);
    gtk_image_set_from_icon_name(GTK_IMAGE(image), icon_name);

    // Return FALSE to stop timeout
    return TRUE;
}

void handle_battery(GtkBuilder *builder) {
    GtkButton *battery_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "battery-button"));

    // First call immediately
    handle_battery_change(battery_button);
    // Then set 1s timeout
    g_timeout_add_seconds(1, handle_battery_change, battery_button);
}
