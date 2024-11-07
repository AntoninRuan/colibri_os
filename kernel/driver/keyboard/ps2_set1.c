#include <kernel/keyboard.h>
#include <stdint.h>

#include "ps2.h"

enum scancode {
    IGNORED,
    SCANCODE_ESC,
    SCANCODE_1,
    SCANCODE_2,
    SCANCODE_3,
    SCANCODE_4,
    SCANCODE_5,
    SCANCODE_6,
    SCANCODE_7,
    SCANCODE_8,
    SCANCODE_9,
    SCANCODE_0,
    SCANCODE_MINUS,
    SCANCODE_EQUALS,
    SCANCODE_BACKSPACE,
    SCANCODE_TAB,
    SCANCODE_Q,
    SCANCODE_W,
    SCANCODE_E,
    SCANCODE_R,
    SCANCODE_T,
    SCANCODE_Y,
    SCANCODE_U,
    SCANCODE_I,
    SCANCODE_O,
    SCANCODE_P,
    SCANCODE_LBRAKET,
    SCANCODE_RBRAKET,
    SCANCODE_RETURN,
    SCANCODE_LCTRL,
    SCANCODE_A,
    SCANCODE_S,
    SCANCODE_D,
    SCANCODE_F,
    SCANCODE_G,
    SCANCODE_H,
    SCANCODE_J,
    SCANCODE_K,
    SCANCODE_L,
    SCANCODE_SEMI_COLON,
    SCANCODE_QUOTE,
    SCANCODE_BACKTICK,
    SCANCODE_LSHIFT,
    SCANCODE_BACKSLASH,
    SCANCODE_Z,
    SCANCODE_X,
    SCANCODE_C,
    SCANCODE_V,
    SCANCODE_B,
    SCANCODE_N,
    SCANCODE_M,
    SCANCODE_COMMA,
    SCANCODE_DOT,
    SCANCODE_SLASH,
    SCANCODE_RSHIFT,
    SCANCODE_KEYPAD_MULT,
    SCANCODE_LALT,
    SCANCODE_SPACE,
    SCANCODE_CAPS_LOCK,
    SCANCODE_F1,
    SCANCODE_F2,
    SCANCODE_F3,
    SCANCODE_F4,
    SCANCODE_F5,
    SCANCODE_F6,
    SCANCODE_F7,
    SCANCODE_F8,
    SCANCODE_F9,
    SCANCODE_F10,
    SCANCODE_NUMLOCK,
    SCANCODE_SCROLL_LOCK,
    SCANCODE_KEYPAD_7,
    SCANCODE_KEYPAD_8,
    SCANCODE_KEYPAD_9,
    SCANCODE_KEYPAD_MINUS,
    SCANCODE_KEYPAD_4,
    SCANCODE_KEYPAD_5,
    SCANCODE_KEYPAD_6,
    SCANCODE_KEYPAD_PLUS,
    SCANCODE_KEYPAD_1,
    SCANCODE_KEYPAD_2,
    SCANCODE_KEYPAD_3,
    SCANCODE_KEYPAD_0,
    SCANCODE_KEYPAD_DOT,
    SCANCODE_F11 = 0x57,
    SCANCODE_F12 = 0x58
};

uint8_t last_scancode = 0;

void handle_set1(uint8_t scancode) {
    if (last_scancode == 0xE0) {

    } else {
        if (scancode == 0xE0) goto end;

        if ((scancode & ~0x80) <= 0x58) {
            enum scancode code = (enum scancode) scancode & ~0x80;

            // Scancode are already equals to keycode
            if (scancode >= 0x80) { // Key released
                key_release(code);
                goto end;
            }

            key_pressed(code);
            goto end;
        }
    }

end:
    last_scancode = scancode;
}
