// MatrixOS Keyboard Driver
// Version 0.4

#define KEYBOARD_PORT 0x60

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

char keyboard_get_char(void)
{
    unsigned char scancode;

    do
    {
        scancode = inb(KEYBOARD_PORT);
    }
    while (scancode == 0);

    return scancode;
}
