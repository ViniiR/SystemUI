#include "brightness.h"
#include <stdio.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <stdbool.h>

const char DBUS_SERVICE_NAME[] = "com.vinii.VGSController";
sd_bus *p_dbus = NULL;

static int fail_with_message(const char message[], sd_bus *p, const int code);

int main(int argc, char **argv) {
    int err = 0;

    err = sd_bus_open_system(&p_dbus);
    if (err < 0)
        return fail_with_message("Failed to connect to bus", p_dbus, err);
    printf("Connected to bus\n");

    err = sd_bus_add_object_vtable(
        p_dbus,
        NULL,
        BRIGHTNESS_PATH,
        BRIGHTNESS_INTERFACE,
        BRIGHTNESS_VTABLE,
        NULL
    );
    if (err < 0)
        return fail_with_message("Failed to add object vtable", p_dbus, err);
    printf("Added '%s' object vtable\n", BRIGHTNESS_PATH);

    err = sd_bus_request_name(p_dbus, DBUS_SERVICE_NAME, 0);
    if (err < 0)
        return fail_with_message("Failed to request service name", p_dbus, err);
    printf("Started with service name: '%s'\n", DBUS_SERVICE_NAME);

    while (true) {
        err = sd_bus_process(p_dbus, NULL);
        if (err < 0)
            return fail_with_message("Failed to process bus", p_dbus, err);

        err = sd_bus_wait(p_dbus, 100);
        if (err < 0)
            return fail_with_message("Failed to wait for bus", p_dbus, err);
    }

    // Finish
    sd_bus_unref(p_dbus);
    return err;
}

static int fail_with_message(const char message[], sd_bus *p, const int code) {
    printf("D-Bus Error: %s %s\n", message, strerror(-code));

    sd_bus_unref(p);
    return code;
}
