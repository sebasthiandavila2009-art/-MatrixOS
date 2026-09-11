// MatrixOS PS/2 Mouse Driver
// Version 0.3 - Mouse Cursor

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

static int mouse_wait_read(void)
{
    unsigned int timeout = 100000;

    while (timeout--)
    {
        unsigned char status = inb(MOUSE_STATUS_PORT);

        if ((status & 1) && (status & 0x20))
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

    /* Enable PS/2 auxiliary mouse device. */
    if (!mouse_wait_write())
        return;

    outb(MOUSE_COMMAND_PORT, 0xA8);

    /* Read controller configuration. */
    if (!mouse_wait_write())
        return;

    outb(MOUSE_COMMAND_PORT, 0x20);

    if (!mouse_read(&status))
        return;

    /* Enable mouse IRQ bit. */
    status |= 0x02;

    /* Enable mouse clock. */
    status &= ~0x20;

    /* Write controller configuration. */
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
 * Read a complete 3-byte PS/2 mouse packet.
 *
 * Returns:
 *   1 = packet received
 *   0 = no packet available
 */
int mouse_get_packet(int *dx, int *dy, unsigned char *buttons)
{
    static unsigned char packet[3];
    static int packet_index = 0;

    unsigned char value;

    if (!mouse_read(&value))
        return 0;

    packet[packet_index++] = value;

    if (packet_index < 3)
        return 0;

    packet_index = 0;

    /*
     * First byte:
     * bit 0 = left button
     * bit 1 = right button
     * bit 2 = middle button
     * bit 3 = always 1
     * bit 4 = X sign
     * bit 5 = Y sign
     * bit 6 = X overflow
     * bit 7 = Y overflow
     */

    *buttons = packet[0] & 0x07;

    *dx = (int)(signed char)packet[1];
    *dy = (int)(signed char)packet[2];

    /*
     * Ignore packets with overflow.
     */
    if (packet[0] & 0xC0)
    {
        *dx = 0;
        *dy = 0;
    }

    return 1;
}
