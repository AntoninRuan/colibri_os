#include <kernel/debug/qemu.h>
#include <kernel/log.h>
#include <stdio.h>

char *level_prefix[4] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
};

bool log_qemu = true;
bool log_tty = false;

void enable_tty_log() { log_tty = true; }
void disable_tty_log() { log_tty = false; }

void log(log_level_t level, char* msg) {
    char prefix[64] = {0};
    sprintf(prefix, "[%s]: ", level_prefix[level]);
    if (log_qemu) {
        qemu_write_string(prefix);
        qemu_write_string(msg);
        qemu_write_string("\n");
    }

    if (log_tty) {
        printf("%s%s\n", prefix, msg);
    }
}
