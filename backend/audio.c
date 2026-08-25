#include "audio.h"
#include "audio_individual.h"
#include "audio_internal.h"
#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>

const char AUDIO_PATH[] = "/com/vinii/vgsc/Audio";
const char AUDIO_INTERFACE[] = "com.vinii.vgsc.Audio";

const sd_bus_vtable AUDIO_VTABLE[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
        "GetAudio",
        "",
        "",
        "ub",
        SD_BUS_PARAM(percentage) SD_BUS_PARAM(is_muted),
        get_audio_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_METHOD_WITH_NAMES(
        "SetAudio",
        "u",
        SD_BUS_PARAM(percentage),
        "ub",
        SD_BUS_PARAM(percentage) SD_BUS_PARAM(is_muted),
        set_audio_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_METHOD_WITH_NAMES(
        "ToggleAudioMuted",
        "",
        "",
        "",
        "",
        toggle_audio_muted_handler,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_METHOD_WITH_NAMES(
        "GetAudioIndividual",
        "",
        "",
        "",
        "",
        get_audio_handler_individual,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_METHOD_WITH_NAMES(
        "SetAudioIndividual",
        "uu",
        SD_BUS_PARAM(percentage) SD_BUS_PARAM(sink_id),
        "",
        "",
        set_audio_handler_individual,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_METHOD_WITH_NAMES(
        "ToggleAudioMutedIndividual",
        "u",
        SD_BUS_PARAM(sink_id),
        "",
        "",
        toggle_audio_muted_handler_individual,
        SD_BUS_VTABLE_UNPRIVILEGED
    ),
    SD_BUS_VTABLE_END
};

static const unsigned int UID = 1000;
static const char DEFAULT_SINK[] = "@DEFAULT_SINK@";

//

static ResultHeapStructPointer get_pipewire_volume(const char *sink);
static ResultVoid set_pipewire_volume(
    unsigned int percentage, const char *sink
);
static ResultBool get_is_pipewire_muted();
static ResultVoid toggle_pipewire_muted(const char *sink);

//

static int get_audio_handler_sink(
    sd_bus_message *p_msg,
    void *p_userdata,
    sd_bus_error *p_reterror,
    const char *sink_id
);
static int set_audio_handler_sink(
    sd_bus_message *p_msg,
    void *p_userdata,
    sd_bus_error *p_reterror,
    const char *sink_id
);
static int toggle_audio_muted_handler_sink(
    sd_bus_message *p_msg,
    void *p_userdata,
    sd_bus_error *p_reterror,
    const char *sink_id
);

//

int get_audio_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return get_audio_handler_sink(p_msg, p_userdata, p_reterror, DEFAULT_SINK);
}
int set_audio_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return set_audio_handler_sink(p_msg, p_userdata, p_reterror, DEFAULT_SINK);
}
int toggle_audio_muted_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return toggle_audio_muted_handler_sink(
        p_msg, p_userdata, p_reterror, DEFAULT_SINK
    );
}

//

int get_audio_handler_individual(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    // TODO: return array here
    // uint32_t value;
    //
    // int res = sd_bus_message_read(p_msg, "u", &value);
    // if (res < 0) {
    //     return sd_bus_error_setf(
    //         p_reterror,
    //         SD_BUS_ERROR_INVALID_ARGS,
    //         "Expected uint32_t sink id"
    //     );
    // }

    // return get_audio_handler_sink(p_msg, p_userdata, p_reterror,
    // DEFAULT_SINK);
    get_all_streams();
    return sd_bus_reply_method_return(p_msg, "", NULL);
}
int set_audio_handler_individual(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return set_audio_handler_sink(p_msg, p_userdata, p_reterror, DEFAULT_SINK);
}
int toggle_audio_muted_handler_individual(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    return toggle_audio_muted_handler_sink(
        p_msg, p_userdata, p_reterror, DEFAULT_SINK
    );
}

//

static int get_audio_handler_sink(
    sd_bus_message *p_msg,
    void *p_userdata,
    sd_bus_error *p_reterror,
    const char *sink_id
) {
    ResultHeapStructPointer result_volume = get_pipewire_volume(sink_id);
    if (result_volume.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to get volume, Error: %s",
            result_volume.err_msg
        );
    }

    VolumeStatus *volume_status = (VolumeStatus *)result_volume.ok_value;

    int return_value = sd_bus_reply_method_return(
        p_msg, "ub", volume_status->volume, volume_status->is_muted
    );

    free(result_volume.ok_value);

    return return_value;
}

static int set_audio_handler_sink(
    sd_bus_message *p_msg,
    void *p_userdata,
    sd_bus_error *p_reterror,
    const char *sink_id
) {
    uint32_t value;
    int res;

    res = sd_bus_message_read(p_msg, "u", &value);
    if (res < 0) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_INVALID_ARGS,
            "Expected uint32_t percentage"
        );
    }

    if (value < 0 || value > 100) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_INVALID_ARGS,
            "Percentage out of bounds '0..=100'"
        );
    }

    ResultVoid result_volume = set_pipewire_volume(value, sink_id);
    if (result_volume.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to set volume, Error: %s",
            result_volume.err_msg
        );
    }

    //

    ResultHeapStructPointer result_get_volume = get_pipewire_volume(sink_id);
    if (result_get_volume.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to get volume, Error: %s",
            result_get_volume.err_msg
        );
    }

    VolumeStatus *volume_status = (VolumeStatus *)result_get_volume.ok_value;

    int return_value = sd_bus_reply_method_return(
        p_msg, "ub", volume_status->volume, volume_status->is_muted
    );

    free(result_get_volume.ok_value);

    return return_value;
}

static int toggle_audio_muted_handler_sink(
    sd_bus_message *p_msg,
    void *p_userdata,
    sd_bus_error *p_reterror,
    const char *sink_id
) {
    ResultVoid result_muted = toggle_pipewire_muted(sink_id);
    if (result_muted.variant == ERR) {
        return sd_bus_error_setf(
            p_reterror,
            SD_BUS_ERROR_FAILED,
            "Failed to toggle volume muted, Error: %s",
            result_muted.err_msg
        );
    }

    return sd_bus_reply_method_return(p_msg, NULL);
}

//

// NOTE: this is the hackiest module in the application, running command as user
// 1000 it's not ideal and likely a temporary solution, as far as temporary
// solutions last


ResultHeapStructPointer external_get_pipewire_volume(const char *sink_id) {
    return get_pipewire_volume(sink_id);
}

static ResultHeapStructPointer get_pipewire_volume(const char *sink_id) {
    ResultHeapStructPointer res = {
        .variant = ERR, .err_msg = RESULT_ERR_MSG_UNKNOWN, .ok_value = NULL
    };

    char command[STRING_KB];
    snprintf(command, sizeof(command), "wpctl get-volume %s", sink_id);

    char exec_output[STRING_KB];
    ResultVoid result_exec = exec_command_as_user(
        exec_output, sizeof(exec_output), command, "r", UID
    );
    if (result_exec.variant == ERR) {
        res.err_msg = result_exec.err_msg;
        return res;
    }

    //

    char volume_input[STRING_KB];
    snprintf(
        volume_input,
        sizeof(volume_input),
        "echo \"%s\" | awk '{printf $2}'",
        exec_output
    );

    char volume_output[STRING_KB];
    ResultVoid result_volume =
        exec_command(volume_output, sizeof(volume_output), volume_input, "r");
    if (result_volume.variant == ERR) {
        res.err_msg = result_volume.err_msg;
        return res;
    }

    //

    int volume = atof(volume_output) * 100.0;

    bool is_muted = false;
    if (strstr(exec_output, "[MUTED]") != NULL) {
        is_muted = true;
    }

    VolumeStatus *status = malloc(sizeof(VolumeStatus));
    if (status == NULL) {
        res.err_msg = "Failed to alloc";
        return res;
    }
    status->is_muted = is_muted;
    status->volume = volume;

    res.variant = OK;
    res.err_msg = "";
    res.ok_value = status;
    return res;
}

static ResultVoid set_pipewire_volume(
    unsigned int percentage, const char *sink
) {
    ResultVoid res = RESULT_VOID_DEFAULT;

    char command[STRING_KB];
    snprintf(
        command, sizeof(command), "wpctl set-volume %s %i%%", sink, percentage
    );

    ResultVoid result_exec = exec_command_as_user(NULL, 0, command, "r", UID);
    if (result_exec.variant == ERR) {
        res.err_msg = result_exec.err_msg;
        return res;
    }

    res.variant = OK;
    res.err_msg = "";
    return res;
}

static ResultVoid toggle_pipewire_muted(const char *sink) {
    ResultVoid res = RESULT_VOID_DEFAULT;

    char command[STRING_KB];
    snprintf(command, sizeof(command), "wpctl set-mute %s toggle", sink);

    ResultVoid result_exec = exec_command_as_user(NULL, 0, command, "r", UID);
    if (result_exec.variant == ERR) {
        res.err_msg = result_exec.err_msg;
        return res;
    }

    res.variant = OK;
    res.err_msg = "";
    return res;
}
