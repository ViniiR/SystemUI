#include "brightness.h"
#include "systemd/sd-bus-protocol.h"
#include "types.h"
#include "util.h"
#include <dirent.h>
#include <math.h>
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
            p_reterror,
            SD_BUS_ERROR_INVALID_ARGS,
            "Expected uint32_t percentage"
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
    ResultInt result = get_brightness();
    if (result.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to get brightness, Error: %s",
            result.err_msg
        );
    }

    return sd_bus_reply_method_return(p_msg, "u", result.ok_value);
}

//

static const char BACKLIGHT_PATH[] = "/sys/class/backlight";
static const char MAX_BRIGHTNESS_PATH[] = "/max_brightness";
static const char CURRENT_BRIGHTNESS_PATH[] = "/brightness";

static ResultInt get_brightness() {
    DIR *dir;
    struct dirent *entry;
    ResultInt res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = 0
    };

    dir = opendir(BACKLIGHT_PATH);
    if (dir == NULL) {
        res.err_msg = "Failed to open directory";
        return res;
    }

    // Operates on the first file it finds that doesn't start with '.'
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *filepath;
        filepath = malloc(PATH_MAX);
        if (filepath == NULL) {
            res.err_msg = "Failed to alloc";
            return res;
        }

        char *max_brightness;
        snprintf(
            filepath,
            PATH_MAX,
            "%s/%s%s",
            BACKLIGHT_PATH,
            entry->d_name,
            MAX_BRIGHTNESS_PATH
        );
        ResultString file_result = read_file(filepath);
        if (file_result.variant == ERR) {
            res.err_msg = file_result.err_msg;
            return res;
        }
        max_brightness = file_result.ok_value;

        char *current_brightness;
        snprintf(
            filepath,
            PATH_MAX,
            "%s/%s%s",
            BACKLIGHT_PATH,
            entry->d_name,
            CURRENT_BRIGHTNESS_PATH
        );
        file_result = read_file(filepath);
        if (file_result.variant == ERR) {
            res.err_msg = file_result.err_msg;
            return res;
        }
        current_brightness = file_result.ok_value;

        int percent;
        percent = (atoi(current_brightness) * 100) / atoi(max_brightness);

        free(filepath);
        free(max_brightness);
        free(current_brightness);

        res.variant = OK;
        res.ok_value = percent;
        return res;
    }

    res.err_msg = "Failed to read backlight directory";
    return res;
}

static ResultVoid set_brightness(
    const unsigned int percent, const char *filepath, const char *max_filepath
) {
    static const int MAX_LEN = sizeof(char) * 50;

    ResultVoid res = RESULT_VOID_DEFAULT;

    ResultString result_max = read_file(max_filepath);
    if (result_max.variant == ERR) {
        res.err_msg = result_max.err_msg;
        return res;
    }

    int calculated_value =
        ceil((float)percent * atoi(result_max.ok_value) / 100);

    char *str = malloc(MAX_LEN);
    snprintf(str, MAX_LEN, "%i", calculated_value);

    ResultVoid result = write_file(filepath, str);
    if (result.variant == ERR) {
        res.err_msg = result.err_msg;
        return res;
    }

    free(str);

    res.variant = OK;
    res.err_msg = "";
    return res;
}
static ResultVoid set_brightness_all(const unsigned int percent) {
    DIR *dir;
    struct dirent *entry;
    ResultVoid res = RESULT_VOID_DEFAULT;

    dir = opendir(BACKLIGHT_PATH);
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
        snprintf(
            filepath,
            PATH_MAX,
            "%s/%s%s",
            BACKLIGHT_PATH,
            entry->d_name,
            CURRENT_BRIGHTNESS_PATH
        );

        char *max_filepath = malloc(PATH_MAX);
        if (max_filepath == NULL) {
            res.err_msg = "Failed to alloc";
            return res;
        }
        snprintf(
            max_filepath,
            PATH_MAX,
            "%s/%s%s",
            BACKLIGHT_PATH,
            entry->d_name,
            MAX_BRIGHTNESS_PATH
        );

        ResultVoid result = set_brightness(percent, filepath, max_filepath);
        if (result.variant == ERR) {
            res.err_msg = result.err_msg;
            return res;
        }

        free(filepath);
    }

    closedir(dir);

    res.variant = OK;
    res.err_msg = "";
    return res;
}
