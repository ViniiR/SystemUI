#include "dirent.h"
#include "util.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkshortcut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char power_supply[] = "/sys/class/power_supply";
static const char capacity[] = "/capacity";
static const char energy_full[] = "/energy_full";
static const char energy_now[] = "/energy_now";
static const char status[] = "/status";

// TODO:
static int battery_dir_name = -1;

void get_battery_directory(char *output, const unsigned int size) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(power_supply);
    if (dir == NULL) {
        safe_fail("Failed to open directory");
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char start[3 + 1];
        snprintf(start, sizeof(start), "%.3s", entry->d_name);

        if (strcmp(start, "BAT") == 0) {
            snprintf(output, size, "%s/%s", power_supply, entry->d_name);

            // Stop at first battery
            return;
        }
    }
    safe_fail("Failed to read directory");
}

int get_battery_percentage() {
    char directory[PATH_MAX];
    get_battery_directory(directory, sizeof(directory));

    char full_capacity[PATH_MAX];
    snprintf(full_capacity, sizeof(full_capacity), "%s%s", directory, energy_full);

    char now_capacity[PATH_MAX];
    snprintf(now_capacity, sizeof(now_capacity), "%s%s", directory, energy_now);

    char *full = read_file(full_capacity);
    int full_number = atoi(full);
    char *now = read_file(now_capacity);
    int now_number = atoi(now);

    int value = (int)round(((double)now_number / (double)full_number) * 100.0);

    free(full);
    free(now);
    return value;
}

bool get_conservation_mode() {
    static const char filepath[] =
        "/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode";

    char *res = read_file(filepath);
    bool is_enabled = false;

    if (atoi(res) == 1) {
        is_enabled = true;
    }

    free(res);
    return is_enabled;
}

typedef enum { CHARGING, DISCHARGING, PLUGGED_IN } ChargingStatus;

ChargingStatus get_charging_status() {
    char directory[PATH_MAX];
    get_battery_directory(directory, sizeof(directory));

    char filepath[PATH_MAX + sizeof(status)];
    snprintf(filepath, sizeof(filepath), "%s%s", directory, status);

    char *output = read_file(filepath);

    ChargingStatus status = CHARGING;
    if (strcmp(output, "Not charging") == 0) {
        status = PLUGGED_IN;
    } else if (strcmp(output, "Discharging") == 0) {
        status = DISCHARGING;
    }

    free(output);
    return status;
}

void get_battery_icon(char *output, const unsigned int size) {
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

gboolean handle_battery_change(void *data) {
    GtkButton *button = GTK_BUTTON(data);
    GtkBox *box = GTK_BOX(gtk_button_get_child(button));

    char icon_name[4000];
    get_battery_icon(icon_name, sizeof(icon_name));

    // Hardcoded
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
