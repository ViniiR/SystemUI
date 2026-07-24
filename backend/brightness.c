#include <systemd/sd-bus.h>
#include "brightness.h"

int set_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return 0;
}
int get_brightness_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return 0;
}
