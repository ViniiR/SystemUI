#include "brightness.h"
#include "systemd/sd-bus-protocol.h"
#include <stdint.h>
#include <systemd/sd-bus.h>

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
            "Percentage out of bounds 0-100"
        );
    }

    return sd_bus_reply_method_return(p_msg, NULL);
}

int get_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return 0;
}
