// MatrixOS PS/2 Mouse Driver
// Version 0.1

#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_COMMAND_PORT 0x64

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

static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static void mouse_wait(unsigned char type)
{
    unsigned int timeout = 100000;

    if (type == 0)
    {
        while (timeout--)
        {
            if (inb(MOUSE_STATUS_PORT) & 1)
                return;
        }
    }
    else
    {
        while (timeout--)
        {
            if (!(inb(MOUSE_STATUS_PORT) & 2))
                return;
        }
    }
}

static void mouse_write(unsigned char value)
{
    mouse_wait(1);

    outb(MOUSE_COMMAND_PORT, 0xD4);

    mouse_wait(1);

    outb(MOUSE_DATA_PORT, value);
}

static unsigned char mouse_read(void)
{
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_init(void)
{
    unsigned char status;

    /* Enable the auxiliary mouse device */
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xA8);

    /* Read controller configuration byte */
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x20);

    status = mouse_read();

    /* Enable mouse interrupts */
    status |= 0x02;

    /* Enable mouse clock */
    status &= ~0x20;

    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x60);

    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);

    /* Tell the mouse to use its default settings */
    mouse_write(0xF6);
    mouse_read();

    /* Enable mouse data reporting */
    mouse_write(0xF4);
    mouse_read();
}

/*
 * Returns one mouse packet byte.
 *
 * This first version is intentionally simple.
 * Interrupt-driven mouse input will be added later.
 */
unsigned char mouse_get_byte(void)
{
    return mouse_read();
}
