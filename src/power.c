#include "util.h"

void handle_shutdown() {
    char res[4000];
    exec_command(res, sizeof(res), "loginctl shutdown", "r");
}
void handle_reboot() {
    char res[4000];
    exec_command(res, sizeof(res), "loginctl reboot", "r");
}
void handle_logou() {
    char res[4000];
    exec_command(res, sizeof(res), "loginctl logout", "r");
}
