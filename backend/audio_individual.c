#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdbool.h>

static const char list_streams_command[] =
    "pw-dump | jq -r '.[] | select(.type == \"PipeWire:Interface:Node\" and "
    "(.info.props[\"media.class\"] | strings | startswith(\"Stream/\") and "
    "contains(\"Audio\"))) | .id'";

static const char inspect_format[] = "wpctl inspect %i | grep \"media.name\"";

typedef struct {
    unsigned int volume;
    char *name;
    bool is_muted;
} AudioStream;

static const unsigned int UID = 1000;

ResultVoid get_all_streams() {
    ResultVoid res = RESULT_VOID_DEFAULT;

    char out[STRING_KB];
    ResultVoid result_command =
        exec_command_as_user(out, sizeof(out), list_streams_command, "r", UID);
    if (result_command.variant == ERR) {
        res.err_msg = result_command.err_msg;
        return res;
    }

    printf("%s\n", out);

    res.variant =OK;
    res.err_msg = "";
    return res;
}
