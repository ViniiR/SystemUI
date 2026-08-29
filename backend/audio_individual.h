#pragma once

#include "audio_internal.h"
#include "types.h"
#include <stddef.h>

ResultVoid get_all_streams(AudioStream **stream_array, size_t *array_size);
