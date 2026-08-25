#include "audio_internal.h"
#include "types.h"
#include "util.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus-protocol.h>

static const char jq_format[] =
    "pw-dump | jq -r '.[] | select(.type == \"PipeWire:Interface:Node\" and "
    "(.info.props[\"media.class\"] | strings | startswith(\"Stream/\") and "
    "contains(\"Audio\"))) | %s '";


static const char inspect_format[] = "wpctl inspect %i | grep \"media.name\"";

ResultHeapStructPointer get_stream(unsigned int id) ;

static const unsigned int UID = 1000;

ResultVoid get_all_streams(AudioStream *stream_array) {
    ResultVoid res = RESULT_VOID_DEFAULT;

    char command[STRING_KB];
    snprintf(command, sizeof(command), jq_format, ".id");

    char output_id[STRING_KB];
    ResultVoid result_ids =
        exec_command_as_user(output_id, sizeof(output_id), command, "r", UID);
    if (result_ids.variant == ERR) {
        res.err_msg = result_ids.err_msg;
        return res;
    }

    FILE *f = fmemopen((void *)output_id, strlen(output_id), "r");
    if (f == NULL) {
        res.err_msg = "fmemopen failed";
        return res;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read_bytes;

    while ((read_bytes = getline(&line, &len, f)) != -1) {
        line[strcspn(line, "\r\n")] = 0;

        ResultHeapStructPointer result = get_stream(atoi(line));
        if (result.variant==ERR) {
            res.err_msg=result.err_msg;
        }
        AudioStream *stream = result.ok_value;
        printf("Vol: %i, Muted: %i, Name: %s", stream->volume, stream->is_muted, stream->name);

        free(result.ok_value);
    }

    free(line);
    fclose(f);

    res.variant = OK;
    res.err_msg = "";
    return res;
}

ResultHeapStructPointer get_stream(unsigned int id) {
    ResultHeapStructPointer res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = NULL
    };

    char id_char[STRING_KB];
    snprintf(id_char, sizeof(id_char), "%i", id);

    char name_command[STRING_KB];
    snprintf(
        name_command,
        sizeof(name_command),
        "wpctl inspect %i | grep media.name | sed "
        "'s/media.name.*\"\\(.*\\)\"/\\1/'",
        id
    );
    char output_name[STRING_KB];
    ResultVoid result_name = exec_command_as_user(
        output_name, sizeof(output_name), name_command, "r", UID
    );
    if (result_name.variant == ERR) {
        res.err_msg = result_name.err_msg;
        return res;
    }

    //

    ResultHeapStructPointer result_volume =
        external_get_pipewire_volume(id_char);
    if (result_volume.variant == ERR) {
        res.err_msg = result_volume.err_msg;
        return res;
    }
    VolumeStatus *status = result_volume.ok_value;

    AudioStream *audio = malloc(sizeof(AudioStream));
    audio->volume = status->volume, audio->is_muted = status->is_muted,
    audio->name = output_name,

    free(result_volume.ok_value);

    res.variant = OK;
    res.err_msg = "";
    res.ok_value = audio; // TODO:
    return res;
}
