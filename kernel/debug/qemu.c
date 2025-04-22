#include <kernel/serial.h>
#include <kernel/debug/qemu.h>

#ifdef __arch_x86_64
#include <kernel/x86-64.h>
#endif

int init_qemu_serial() { return init_serial(QEMU_SERIAL_PORT); }

void qemu_write_char(const char ch) {
    while((inb(QEMU_SERIAL_PORT + 5) & 0x20) == 0);
    outb(QEMU_SERIAL_PORT, ch);
}

void qemu_write_string(const char *string) {
    while (*string != '\0'){
        qemu_write_char(*string);
        string++;
    }
}
