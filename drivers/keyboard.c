// MatrixOS Keyboard Driver
// Version 0.6 - Keyboard/Mouse Separation

#define KEYBOARD_DATA    0x60
#define KEYBOARD_STATUS  0x64

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}


/*
 * Read keyboard input without blocking.
 *
 * PS/2 controller status:
 *
 * Bit 0 = output buffer has data
 * Bit 5 = data came from mouse
 *
 * We only accept data when:
 *
 * Bit 0 = 1
 * Bit 5 = 0
 */
char keyboard_get_char(void)
{
    unsigned char status;
    unsigned char scancode;

    status = inb(KEYBOARD_STATUS);

    /*
     * No data available.
     */
    if ((status & 0x01) == 0)
        return 0;

    /*
     * Data came from the mouse.
     *
     * DO NOT read it as keyboard input.
     */
    if (status & 0x20)
        return 0;

    /*
     * Now we know the byte is keyboard data.
     */
    scancode = inb(KEYBOARD_DATA);

    /*
     * Ignore key-release codes.
     */
    if (scancode & 0x80)
        return 0;


    /*
     * Number row.
     */

    switch (scancode)
    {
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';


        /*
         * QWERTY row.
         */

        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';


        /*
         * ASDF row.
         */

        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';


        /*
         * ZXCV row.
         */

        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';


        /*
         * Space.
         */

        case 0x39:
            return ' ';


        /*
         * Enter.
         */

        case 0x1C:
            return '\n';


        /*
         * Backspace.
         */

        case 0x0E:
            return '\b';


        /*
         * Unknown key.
         */

        default:
            return 0;
    }
}
