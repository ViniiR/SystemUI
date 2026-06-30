#pragma once

#include <gtk/gtk.h>

typedef enum { CHARGING, DISCHARGING, PLUGGED_IN } ChargingStatus;

typedef struct {
    GDBusConnection *connection;
} State;
