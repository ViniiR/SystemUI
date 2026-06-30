#pragma once

#include <gtk/gtk.h>
#include "types.h"

void handle_brightness(GtkBuilder *builder, State *state);

void handle_audio(GtkBuilder *builder);

void handle_power_buttons(GtkBuilder *builder);

void handle_battery(GtkBuilder *builder);

void handle_conservation_mode(GtkBuilder *builder);

void handle_boost_mode(GtkBuilder *builder);
