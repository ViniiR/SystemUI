#include "boost.h"
#include "types.h"
#include "util.h"
#include <stdbool.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

const char BOOST_PATH[] = "/com/vinii/vgsc/Boost";
const char BOOST_INTERFACE[] = "com.vinii.vgsc.Boost";

const sd_bus_vtable BOOST_VTABLE[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
        "ToggleBoost",
        "",
        "",
        "",
        "",
        toggle_boost_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_VTABLE_END
};

//

static ResultBool get_is_boost_active();
static ResultVoid set_boost_mode(bool value);

//

int toggle_boost_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    ResultBool result_is_active = get_is_boost_active();
    if (result_is_active.variant ==ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to get boost, Error: %s",
            result_is_active.err_msg
        );
    }

    ResultVoid result_set_boost = set_boost_mode(!result_is_active.ok_value);
    if (result_set_boost.variant ==ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to set boost, Error: %s",
            result_set_boost.err_msg
        );
    }

    return sd_bus_reply_method_return(p_msg, NULL);
}

//

static const char boost_filepath[] = "/sys/devices/system/cpu/cpufreq/boost";
static const char conservation_filepath[] =
    "/sys/bus/platform/drivers/ideapad_acpi/VPC2004:00/conservation_mode";

static ResultBool get_is_boost_active() {
    ResultBool res = {.variant = ERR, .err_msg = "", .ok_value = false};

    ResultHeapString result_file = read_file(boost_filepath);
    if (result_file.variant == ERR) {
        res.err_msg = result_file.err_msg;
        return res;
    }

    bool is_active = false;
    if (strcmp(result_file.ok_value, "1") == 0) {
        is_active = true;
    }

    free(result_file.ok_value);

    res.variant = OK;
    res.ok_value = is_active;
    return res;
}

static ResultVoid set_boost_mode(bool value) {
    ResultVoid res = RESULT_VOID_DEFAULT;

    char *command;
    int size = asprintf(&command, "echo %i | tee %s > /dev/null", value, boost_filepath);
    if (size == -1) {
        res.err_msg = "Failed to alloc";
        return res;
    }

    ResultVoid result_command = exec_command(NULL, 0, command, "r");
    if (result_command.variant == ERR) {
        free(command);
        res.err_msg = result_command.err_msg;
        return res;
    }

    free(command);

    res.variant = OK;
    return res;
}
