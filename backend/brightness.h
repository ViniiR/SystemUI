#pragma once

#include <systemd/sd-bus.h>

const char BRIGHTNESS_PATH[] = "/com/vinii/VGSController/Brightness";
const char BRIGHTNESS_INTERFACE[] = "com.vinii.VGSController.Brightness";

int set_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
);
int get_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
);

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
