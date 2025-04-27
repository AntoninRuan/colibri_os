#include <kernel/kernel.h>
#include <kernel/log.h>
#include <stdint.h>

__attribute__((__noreturn__)) void panic(char *msg) {
    log(ERROR, msg);
    while (1);
    __builtin_unreachable();
}

kernel_status_t kernel_status = {0};
cpu_status_t cpu_status[MAX_CORES] = {0};

void push_off() {
    cpu_status_t *cpu = get_cpu();
    if(cpu->int_on) {
        cpu->int_on = 0;
        disable_interrupt();
    }

    cpu->push_off_count ++;
}

void pop_off() {
    cpu_status_t *cpu = get_cpu();
    if(cpu->int_on)
        panic("pop_off: cpu interrupt already on");

    cpu->push_off_count --;
    if (cpu->push_off_count == 0) {
        cpu->int_on = 1;
        enable_interrupt();
    }
}

void main(void) { while (1); }
