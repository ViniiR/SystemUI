#include "audio.h"
#include <systemd/sd-bus.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
        "",
        "",
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
    SD_BUS_VTABLE_END
};

//

static int set_pipewire_volume(uid_t target_uid, const char *volume_str);

//

int get_audio_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {

    return sd_bus_reply_method_return(p_msg, NULL);
}

int set_audio_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {
    // TODO: WORKS
    set_pipewire_volume(1000, "90%");

    return sd_bus_reply_method_return(p_msg, NULL);
}

int toggle_audio_muted_handler(
    sd_bus_message *p_msg, void *p_userdata, sd_bus_error *p_reterror
) {

    return sd_bus_reply_method_return(p_msg, NULL);
}

//

// TODO: fix ai generated code
static int set_pipewire_volume(uid_t target_uid, const char *volume_str) {
    pid_t pid = fork();
    if (pid == 0) {
        char runtime_dir[64];
        snprintf(runtime_dir, sizeof(runtime_dir), "/run/user/%d", target_uid);

        setenv("XDG_RUNTIME_DIR", runtime_dir, 1);

        if (setgid(target_uid) != 0 || setuid(target_uid) != 0) {
            perror("Failed to drop privileges");
            exit(1);
        }

        execlp(
            "wpctl",
            "wpctl",
            "set-volume",
            "@DEFAULT_AUDIO_SINK@",
            volume_str,
            NULL
        );
        perror("execlp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
    }
    return -1;
}
