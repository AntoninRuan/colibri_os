#include <kernel/debug/qemu.h>
#include <kernel/serial.h>
#include <kernel/sync.h>

#ifdef __arch_x86_64
#include <kernel/x86-64.h>
#endif

spinlock_t qemu_serial_lock = {
    .held = 0,
    .name = "QEMU serial"
};

int init_qemu_serial() { return init_serial(QEMU_SERIAL_PORT); }

void qemu_write_char(const char ch) {
    while ((inb(QEMU_SERIAL_PORT + 5) & 0x20) == 0);
    outb(QEMU_SERIAL_PORT, ch);
}

void qemu_print(const char *string) {
    acquire(&qemu_serial_lock);
    while (*string != '\0') {
        qemu_write_char(*string);
        string++;
    }
    release(&qemu_serial_lock);
}
