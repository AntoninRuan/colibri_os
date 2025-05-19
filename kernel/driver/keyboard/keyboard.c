#include <kernel/keyboard.h>
#include <kernel/x86.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ps2.h"

#define KEYBOARD_PORT   0x60
#define CONTROLLER_PORT 0x64
#define MAX_RETRY       3

#define SCANCODE_SET_1 0x43
#define SCANCODE_SET_2 0x41
#define SCANCODE_SET_3 0x3F

u8 scancode_set = 0;

u8 keycode_to_ascii[] = {
    0,     // KEY_RESERVED
    0,     // KEY_ESC
    '1',   // KEY_1
    '2',   // KEY_2
    '3',   // KEY_3
    '4',   // KEY_4
    '5',   // KEY_5
    '6',   // KEY_6
    '7',   // KEY_7
    '8',   // KEY_8
    '9',   // KEY_9
    '0',   // KEY_0
    '-',   // KEY_MINUS
    '+',   // KEY_EQUAL
    '\b',  // KEY_BACKSPACE
    '\t',  // KEY_TAB
    'q',   // KEY_Q
    'w',   // KEY_W
    'e',   // KEY_E
    'r',   // KEY_R
    't',   // KEY_T
    'y',   // KEY_Y
    'u',   // KEY_U
    'i',   // KEY_I
    'o',   // KEY_O
    'p',   // KEY_P
    ']',   // KEY_LEFTBRACE
    '[',   // KEY_RIGHTBRACE
    '\n',  // KEY_ENTER
    0,     // KEY_LEFTCTRL
    'a',   // KEY_A
    's',   // KEY_S
    'd',   // KEY_D
    'f',   // KEY_F
    'g',   // KEY_G
    'h',   // KEY_H
    'j',   // KEY_J
    'k',   // KEY_K
    'l',   // KEY_L
    ';',   // KEY_SEMICOLON
    '\'',  // KEY_APOSTROPHE
    '`',   // KEY_GRAVE
    0,     // KEY_LEFTSHIFT
    '\\',  // KEY_BACKSLASH
    'z',   // KEY_Z
    'x',   // KEY_X
    'c',   // KEY_C
    'v',   // KEY_V
    'b',   // KEY_B
    'n',   // KEY_N
    'm',   // KEY_M
    ',',   // KEY_COMMA
    '.',   // KEY_DOT
    '/',   // KEY_SLASH
    0,     // KEY_RIGHTSHIFT
    '*',   // KEY_KPASTERISK
    0,     // KEY_LEFTALT
    ' ',   // KEY_SPACE
    0,     // KEY_CAPSLOCK
    0,     // KEY_F1
    0,     // KEY_F2
    0,     // KEY_F3
    0,     // KEY_F4
    0,     // KEY_F5
    0,     // KEY_F6
    0,     // KEY_F7
    0,     // KEY_F8
    0,     // KEY_F9
    0,     // KEY_F10
    0,     // KEY_NUMLOCK
    0,     // KEY_SCROLLLOCK
    '7',   // KEY_KP7
    '8',   // KEY_KP8
    '9',   // KEY_KP9
    '-',   // KEY_KPMINUS
    '4',   // KEY_KP4
    '5',   // KEY_KP5
    '6',   // KEY_KP6
    '+',   // KEY_KPPLUS
    '1',   // KEY_KP1
    '2',   // KEY_KP2
    '3',   // KEY_KP3
    '0',   // KEY_KP0
    '.',   // KEY_KPDOT
};

void init_keyboard() {
    if (scancode_set != 0) return;

    u8 sc;
    u32 retry = 0;
    do {
        outb(KEYBOARD_PORT, 0xF0);
        outb(KEYBOARD_PORT, 0);

        sc = inb(KEYBOARD_PORT);
    } while (sc == 0xFE && retry++ < MAX_RETRY);

    scancode_set = inb(KEYBOARD_PORT);

    outb(CONTROLLER_PORT, 0x20);
    sc = inb(KEYBOARD_PORT);

    if (sc & (1 << 6)) {  // Translation to set 1 enabled
        scancode_set = SCANCODE_SET_1;
    }
}

void keystroke_handler() {
    u8 scancode = inb(KEYBOARD_PORT);

    switch (scancode_set) {
        case SCANCODE_SET_1:
            handle_set1(scancode);
            break;
        default:
            break;
    }
    return;
}

void key_pressed(u32 code) {
    if (code <= 83) {
        printf("%c", keycode_to_ascii[code]);
    }
    return;
}

void key_release(u32 _) { return; }
