#include "battery.h"
#include "dirent.h"
#include "systemd/sd-bus-vtable.h"
#include "types.h"
#include "util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

const char BATTERY_PATH[] = "/com/vinii/vgsc/Battery";
const char BATTERY_INTERFACE[] = "com.vinii.vgsc.Battery";

// TODO: use SD_BUS_METHOD_WITH_ARGS on all SD_BUS_METHOD_WITH_NAMES
const sd_bus_vtable BATTERY_VTABLE[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
        "GetBattery",
        "",
        "",
        "su",
        SD_BUS_PARAM(icon) SD_BUS_PARAM(percentage),
        get_battery_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_VTABLE_END
};

//

static ResultHeapString get_battery_icon();
static ResultInt get_battery_percentage();

//

int get_battery_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    ResultHeapString result_icon = get_battery_icon();
    if (result_icon.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_INVALID_ARGS,
            "Failed to get battery, Error %s",
            result_icon.err_msg
        );
    }
    ResultInt result_percent = get_battery_percentage();
    if (result_percent.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_INVALID_ARGS,
            "Failed to get battery, Error %s",
            result_percent.err_msg
        );
    }

    int return_value = sd_bus_reply_method_return(
        p_msg, "su", result_icon.ok_value, result_percent.ok_value
    );

    free(result_icon.ok_value);
    return return_value;
}

//

static const char power_supply[] = "/sys/class/power_supply";
static const char capacity[] = "/capacity";
static const char energy_full[] = "/energy_full";
static const char energy_now[] = "/energy_now";
static const char status[] = "/status";

typedef enum { CHARGING, DISCHARGING, PLUGGED_IN } ChargingStatus;

//

static ResultHeapString get_battery_directory() {
    DIR *dir;
    struct dirent *entry;
    ResultHeapString res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = ""
    };

    dir = opendir(power_supply);
    if (dir == NULL) {
        res.err_msg = "Failed to open directory";
        return res;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char start[3 + 1];
        snprintf(start, sizeof(start), "%.3s", entry->d_name);

        if (strcmp(start, "BAT") == 0) {
            char *buf = malloc(4096);
            snprintf(buf, 4096, "%s/%s", power_supply, entry->d_name);

            res.variant = OK;
            res.ok_value = buf;
            return res;
        }
    }

    res.err_msg = "Failed to read directory";
    return res;
}

static ResultInt get_battery_percentage() {
    ResultInt res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = 0
    };

    ResultHeapString result_directory = get_battery_directory();
    if (result_directory.variant == ERR) {
        res.err_msg = result_directory.err_msg;
        return res;
    }

    char full_capacity[4096];
    snprintf(
        full_capacity,
        sizeof(full_capacity),
        "%s%s",
        result_directory.ok_value,
        energy_full
    );

    char now_capacity[4096];
    snprintf(
        now_capacity,
        sizeof(now_capacity),
        "%s%s",
        result_directory.ok_value,
        energy_now
    );

    ResultHeapString result_full = read_file(full_capacity);
    if (result_full.variant == ERR) {
        res.err_msg = result_full.err_msg;
        return res;
    }
    int full_number = atoi(result_full.ok_value);

    ResultHeapString result_now = read_file(now_capacity);
    if (result_now.variant == ERR) {
        res.err_msg = result_now.err_msg;
        return res;
    }
    int now_number = atoi(result_now.ok_value);

    int value = (int)round(((double)now_number / (double)full_number) * 100.0);

    free(result_directory.ok_value);
    free(result_full.ok_value);
    free(result_now.ok_value);

    res.variant = OK;
    res.ok_value = value;
    return res;
}

/// Returns integer of enum ChargingStatus,
/// it is safe to cast ok value to ChargingStatus, unless it is ERR
static ResultInt get_charging_status() {
    ResultInt res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = 0
    };

    ResultHeapString result_dir = get_battery_directory();
    if (result_dir.variant == ERR) {
        res.err_msg = result_dir.err_msg;
        return res;
    }

    char filepath[4096 + sizeof(status)];
    snprintf(filepath, sizeof(filepath), "%s%s", result_dir.ok_value, status);

    ResultHeapString result = read_file(filepath);
    if (result.variant == ERR) {
        res.err_msg = result.err_msg;
        return res;
    }

    ChargingStatus status = CHARGING;
    if (strcmp(result.ok_value, "Not charging") == 0) {
        status = PLUGGED_IN;
    } else if (strcmp(result.ok_value, "Discharging") == 0) {
        status = DISCHARGING;
    }

    free(result_dir.ok_value);
    free(result.ok_value);

    res.variant = OK;
    res.ok_value = status;
    return res;
}

static int format_percentage(const int percentage) {
    int battery_level_formatted = 0;

    if (percentage <= 0) {
        battery_level_formatted = 0;
    } else if (percentage <= 9) {
        battery_level_formatted = percentage;
    } else if (percentage >= 100) {
        battery_level_formatted = 100;
    } else {
        char first[1 + 1];
        snprintf(first, sizeof(first), "%1.0f", (float)percentage);
        battery_level_formatted = atoi(first) * 10;
    }

    return battery_level_formatted;
}

static ResultHeapString allocate_suffix(const ChargingStatus status) {
    static const char plugged[] = "-plugged-in";
    static const char charging[] = "-charging";

    ResultHeapString res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = ""
    };

    int alloc_size = 1;
    char *suffix;
    switch (status) {
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
        alloc_size += 1;
        suffix = malloc(alloc_size);
        snprintf(suffix, alloc_size, "");
        break;
    }

    if (suffix == NULL) {
        res.err_msg = "Failed to alloc";
        return res;
    }

    res.variant = OK;
    res.ok_value = suffix;
    return res;
}

static ResultHeapString get_battery_icon() {
    static const char battery_prefix[] = "battery-level-";

    ResultHeapString res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = ""
    };

    ResultInt charging_status_result = get_charging_status();
    if (charging_status_result.variant == ERR) {
        res.err_msg = charging_status_result.err_msg;
        return res;
    }

    ResultHeapString result_suffix =
        allocate_suffix((ChargingStatus)charging_status_result.ok_value);
    if (result_suffix.variant == ERR) {
        res.err_msg = result_suffix.err_msg;
        return res;
    }

    ResultInt result_percentage = get_battery_percentage();
    if (result_percentage.variant == ERR) {
        res.err_msg = result_percentage.err_msg;
        return res;
    }

    int battery_level_formatted = format_percentage(result_percentage.ok_value);

    char suffix_len = strlen(result_suffix.ok_value);

    int inner_len =
        (sizeof(battery_prefix) + sizeof(battery_level_formatted) +
         sizeof(char) * suffix_len);

    char *buf = malloc(4096);
    snprintf(
        buf,
        sizeof(buf) > inner_len ? sizeof(buf) : inner_len,
        "%s%i%s",
        battery_prefix,
        battery_level_formatted,
        result_suffix.ok_value
    );

    free(result_suffix.ok_value);

    res.variant = OK;
    res.ok_value = buf;
    return res;
}
