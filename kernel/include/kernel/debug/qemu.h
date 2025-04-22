#ifndef QEMU_H
#define QEMU_H

#define QEMU_SERIAL_PORT 0x3F8

int init_qemu_serial();
void qemu_write_char(const char ch);
void qemu_write_string(const char *string);

#endif // QEMU_H
