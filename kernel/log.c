#include <stdio.h>
#include <stdarg.h>

#include <kernel/debug/qemu.h>
#include <kernel/log.h>

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

void log(log_level_t level, const char* msg) {
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

void logf(log_level_t level, const char* msg, ...) {
    va_list ap;
    char final[512] = {0};

    va_start(ap, msg);
    vsprintf(final, msg, ap);
    log(level, final);
    va_end(ap);
}
