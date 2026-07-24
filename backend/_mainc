#include "types.h"
#include "util.h"

int main() {}

static const char boost_filepath[] = "/sys/devices/system/cpu/cpufreq/boost";
static const char conservation_filepath[] =
    "/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode";

static bool get_is_boost_active() {
    char *res = read_file(boost_filepath);
    bool is_active = false;

    if (strcmp(res, "1") == 0) {
        is_active = true;
    }

    free(res);
    return is_active;
}

static void set_boost_mode(bool value) {
    char command[PATH_MAX];

    snprintf(
        command,
        sizeof(command),
        "echo %i | tee %s > /dev/null",
        0,
        boost_filepath
    );

    char res[PATH_MAX];

    exec_command(res, sizeof(res), command, "r");

    printf("%s", res);
}

//

static bool get_is_conservation_active() {
    char *res = read_file(conservation_filepath);
    bool is_active = false;
    if (strcmp(res, "1") == 0) {
        is_active = true;
    }

    free(res);
    return is_active;
}

static void set_conservation_mode(const bool value) {
    char command[PATH_MAX];

    snprintf(
        command,
        sizeof(command),
        "echo %i | tee %s > /dev/null",
        value,
        conservation_filepath
    );

    exec_command(NULL, 0, command, "r");
}

//
static const char power_supply[] = "/sys/class/power_supply";
static const char capacity[] = "/capacity";
static const char energy_full[] = "/energy_full";
static const char energy_now[] = "/energy_now";
static const char status[] = "/status";

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
    snprintf(
        full_capacity, sizeof(full_capacity), "%s%s", directory, energy_full
    );

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

//

static void handle_reboot(GtkButton *button, gpointer data) {
    char res[4000];
    exec_command(res, sizeof(res), "systemctl reboot", "r");
}

static void handle_shutdown(GtkButton *button, gpointer data) {
    char res[4000];
    exec_command(res, sizeof(res), "systemctl poweroff", "r");
}
static void handle_logout() {
    char res[4000];
    perror("BRUV INNNIT");
    // TODO: might be DE/WM specific
    // exec_command(res, sizeof(res), "systemctl ", "r");
}

//

static int get_volume_percent() {
    char output[4000];
    exec_command(
        output,
        sizeof(output),
        "wpctl get-volume @DEFAULT_SINK@ | awk '{print $2}'",
        "r"
    );

    float res = atof(output) * 100.0;

    return res;
}

static void set_volume(const int percent) {
    int value = percent;
    if (value > 100) {
        value = 100;
    } else if (value < 0) {
        value = 0;
    }

    char command[PATH_MAX];
    snprintf(command, PATH_MAX, "wpctl set-volume @DEFAULT_SINK@ %i%%", value);

    exec_command(NULL, 0, command, "r");
}

static bool get_is_muted() {
    char output[4000];
    exec_command(
        output,
        sizeof(output),
        "wpctl get-volume @DEFAULT_SINK@ | awk '{print $3}'",
        "r"
    );

    if (strcmp(output, "[MUTED]") == 0) {
        return true;
    }

    return false;
}

//

// TODO: wont work in sudo
static int get_brightness() {
    char brightness[4000];
    exec_command(brightness, sizeof(brightness), "brightnessctl get", "r");
    char max_brightness[4000];
    exec_command(max_brightness, sizeof(brightness), "brightnessctl max", "r");

    double percentage = (atof(brightness) / atof(max_brightness) * 100);

    return (int)percentage;
}

static void set_brightness(const int percent) {
    char command[PATH_MAX];
    snprintf(command, PATH_MAX, "brightnessctl set %i%%", percent);

    char res[4000];
    exec_command(res, sizeof(res), command, "r");
}
