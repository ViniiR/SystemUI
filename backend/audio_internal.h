#pragma once

#include "types.h"

ResultHeapStructPointer external_get_pipewire_volume(const char *sink_id);

typedef struct {
    bool is_muted;
    int volume;
} VolumeStatus;

typedef struct {
    unsigned int volume;
    char *name;
    bool is_muted;
} AudioStream;

