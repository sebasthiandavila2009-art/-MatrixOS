// MatrixOS PS/2 Mouse Driver
// Version 0.5 - Non-Blocking Mouse

#define MOUSE_DATA_PORT   0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static int wait_write(void)
{
    unsigned int timeout = 100000;
    while (timeout--)
    {
        if ((inb(MOUSE_STATUS_PORT) & 2) == 0)
            return 1;
    }
    return 0;
}

static int wait_controller_read(void)
{
    unsigned int timeout = 100000;
    while (timeout--)
    {
        if (inb(MOUSE_STATUS_PORT) & 1)
            return 1;
    }
    return 0;
}

static void mouse_command(unsigned char command)
{
    if (!wait_write()) return;
    outb(MOUSE_COMMAND_PORT, 0xD4);
    if (!wait_write()) return;
    outb(MOUSE_DATA_PORT, command);
}

static void discard_mouse_response(void)
{
    unsigned int timeout = 10000;
    while (timeout--)
    {
        unsigned char status = inb(MOUSE_STATUS_PORT);
        if ((status & 1) == 0)
            return;
        if (status & 0x20)
            (void)inb(MOUSE_DATA_PORT);
        else
            return;
    }
}

void mouse_init(void)
{
    unsigned char status;

    /* Enable the PS/2 auxiliary device. */
    if (!wait_write()) return;
    outb(MOUSE_COMMAND_PORT, 0xA8);

    /* Read controller configuration. */
    if (!wait_write()) return;
    outb(MOUSE_COMMAND_PORT, 0x20);
    if (!wait_controller_read()) return;
    status = inb(MOUSE_DATA_PORT);

    /* Enable the mouse clock and IRQ bit. */
    status |= 0x02;
    status &= (unsigned char)~0x20;

    if (!wait_write()) return;
    outb(MOUSE_COMMAND_PORT, 0x60);
    if (!wait_write()) return;
    outb(MOUSE_DATA_PORT, status);

    /* Disable mouse reporting while configuring it. */
    mouse_command(0xF5);
    discard_mouse_response();

    /* Set defaults. */
    mouse_command(0xF6);
    discard_mouse_response();

    /* Enable movement reporting. */
    mouse_command(0xF4);
    discard_mouse_response();
}

int mouse_get_packet(int *dx, int *dy, unsigned char *buttons)
{
    static unsigned char packet[3];
    static int packet_index = 0;
    unsigned char value;
    unsigned char status = inb(MOUSE_STATUS_PORT);

    /* Never block the kernel waiting for the mouse. */
    if ((status & 1) == 0 || (status & 0x20) == 0)
        return 0;

    value = inb(MOUSE_DATA_PORT);

    if (packet_index == 0 && (value & 0x08) == 0)
        return 0;

    packet[packet_index++] = value;

    if (packet_index < 3)
        return 0;

    packet_index = 0;

    *buttons = packet[0] & 0x07;
    *dx = (int)(signed char)packet[1];
    *dy = (int)(signed char)packet[2];

    if (packet[0] & 0xC0)
    {
        *dx = 0;
        *dy = 0;
    }

    return 1;
}
