#include <kernel/debug/qemu.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <stdarg.h>
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

void log(log_level_t level, const char *msg) {
    if (!(log_qemu || log_tty)) return;
    char prefix[32] = {0};
    sprintf(prefix, "[CPU #%d] [%s]: ", get_cpu()->id, level_prefix[level]);
    if (log_qemu) {
        char fmsg[200] = {0};
        sprintf(fmsg, "%s%s\n", prefix, msg);
        qemu_print(fmsg);
    }

    if (log_tty) {
        printf("%s%s\n", prefix, msg);
    }
}

void logf(log_level_t level, const char *msg, ...) {
    if (!(log_qemu || log_tty)) return;
    va_list ap;
    char final[128] = {0};

    va_start(ap, msg);
    vsprintf(final, msg, ap);
    log(level, final);
    va_end(ap);
}
