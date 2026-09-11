// MatrixOS PS/2 Mouse Driver
// Version 0.2

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

/* Wait for the controller input buffer to become empty. */
static int mouse_wait_write(void)
{
    unsigned int timeout = 100000;

    while (timeout--)
    {
        if ((inb(MOUSE_STATUS_PORT) & 2) == 0)
            return 1;
    }

    return 0;
}

/* Wait for data from the controller. */
static int mouse_wait_read(void)
{
    unsigned int timeout = 100000;

    while (timeout--)
    {
        if (inb(MOUSE_STATUS_PORT) & 1)
            return 1;
    }

    return 0;
}

static int mouse_write(unsigned char value)
{
    if (!mouse_wait_write())
        return 0;

    outb(MOUSE_COMMAND_PORT, 0xD4);

    if (!mouse_wait_write())
        return 0;

    outb(MOUSE_DATA_PORT, value);

    return 1;
}

static int mouse_read(unsigned char *value)
{
    if (!mouse_wait_read())
        return 0;

    *value = inb(MOUSE_DATA_PORT);
    return 1;
}

void mouse_init(void)
{
    unsigned char status;
    unsigned char response;

    /* Enable the PS/2 auxiliary mouse device. */
    if (!mouse_wait_write())
        return;

    outb(MOUSE_COMMAND_PORT, 0xA8);

    /* Read controller configuration byte. */
    if (!mouse_wait_write())
        return;

    outb(MOUSE_COMMAND_PORT, 0x20);

    if (!mouse_read(&status))
        return;

    /* Enable mouse IRQ bit. */
    status |= 0x02;

    /* Enable mouse clock. */
    status &= ~0x20;

    /* Write controller configuration byte. */
    if (!mouse_wait_write())
        return;

    outb(MOUSE_COMMAND_PORT, 0x60);

    if (!mouse_wait_write())
        return;

    outb(MOUSE_DATA_PORT, status);

    /* Set mouse defaults. */
    if (mouse_write(0xF6))
    {
        mouse_read(&response);
    }

    /* Enable mouse data reporting. */
    if (mouse_write(0xF4))
    {
        mouse_read(&response);
    }
}

/*
 * Read one byte from the mouse.
 * Returns 0 if no data is currently available.
 */
unsigned char mouse_get_byte(void)
{
    unsigned char value;

    if (!mouse_read(&value))
        return 0;

    return value;
}
