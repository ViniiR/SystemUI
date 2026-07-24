#include "brightness.h"
#include "systemd/sd-bus-protocol.h"
#include "types.h"
#include "util.h"
#include <dirent.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

const char BRIGHTNESS_PATH[] = "/com/vinii/VGSController/Brightness";
const char BRIGHTNESS_INTERFACE[] = "com.vinii.VGSController.Brightness";

const sd_bus_vtable BRIGHTNESS_VTABLE[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
        "SetBrightness",
        "u",
        SD_BUS_PARAM(percentage),
        "",
        SD_BUS_PARAM(),
        set_brightness_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_METHOD_WITH_NAMES(
        "GetBrightness",
        "",
        SD_BUS_PARAM(),
        "u",
        SD_BUS_PARAM(percentage),
        get_brightness_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_VTABLE_END
};

//

static ResultInt get_brightness();

static ResultVoid set_brightness_all(const unsigned int percent);

//

int set_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    uint32_t value;
    int res;

    res = sd_bus_message_read(p_msg, "u", &value);
    if (res < 0) {
        return sd_bus_error_setf(
            p_reterror, SD_BUS_ERROR_INVALID_ARGS, "Expected uint32_t value"
        );
    }

    if (value < 0 || value > 100) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_INVALID_ARGS,
            "Percentage out of bounds '0..=100'"
        );
    }

    ResultVoid result = set_brightness_all(value);
    if (result.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to set brightness, Error: %s",
            result.err_msg
        );
    }

    return sd_bus_reply_method_return(p_msg, NULL);
}

int get_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    // TODO:
    return 0;
}

//

static const char PATH[] = "/sys/class/backlight";
static const char MAX[] = "/max_brightness";
static const char BRIGHTNESS[] = "/brightness";

static ResultInt get_brightness() {
    DIR *dir;
    struct dirent *entry;
    ResultInt res = {.variant = ERR, .err_msg = NULL, .ok_value = NULL};

    dir = opendir(PATH);
    if (dir == NULL) {
        res.err_msg = "Failed to open directory";
        return res;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            res.err_msg = "Failed to alloc";
            return res;
        }

        // Operate on the first file it finds
        snprintf(filepath, PATH_MAX, "%s/%s%s", PATH, entry->d_name, MAX);
        char *max_brightness = read_file(filepath);
        free(filepath);

        filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            res.err_msg = "Failed to alloc";
            return res;
        }
        snprintf(filepath, PATH_MAX, "%s/%s%s", PATH, entry->d_name, MAX);
        char *brightness = read_file(filepath);

        int percent = atoi(brightness) / atoi(max_brightness) * 100;

        free(filepath);
        free(max_brightness);
        free(brightness);

        res.variant = OK;
        res.ok_value = percent;
        return res;
    }

    res.variant = OK;
    res.ok_value = 0;
    return res;
}

static void set_brightness(const unsigned int percent, const char *filepath) {
    static const int MAX_LEN = sizeof(char) * 50;

    char *str = malloc(MAX_LEN);
    snprintf(str, MAX_LEN, "%i", percent);

    write_file(filepath, str);
    free(str);
}

static ResultVoid set_brightness_all(const unsigned int percent) {
    DIR *dir;
    struct dirent *entry;
    ResultVoid res = {.variant = ERR, .err_msg = NULL, .ok_value = NULL};

    dir = opendir(PATH);
    if (dir == NULL) {
        res.err_msg = "Failed to open directory.";
        return res;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            res.err_msg = "Failed to alloc";
            return res;
        }

        snprintf(
            filepath, PATH_MAX, "%s/%s%s", PATH, entry->d_name, BRIGHTNESS
        );

        set_brightness(percent, filepath);
        free(filepath);
    }

    closedir(dir);

    res.variant = OK;
    return res;
}
