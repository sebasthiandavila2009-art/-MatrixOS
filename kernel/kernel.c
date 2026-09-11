// MatrixOS Kernel
// Version 0.5

extern char keyboard_get_char(void);
extern void mouse_init(void);

volatile unsigned short *video = (unsigned short *)0xB8000;

int cursor = 0;

void kernel_main(void)
{
    const char *message = "MATRIXOS v0.5 - MOUSE READY";

    /* Initialize the mouse */
    mouse_init();

    /* Display startup message */
    for (int i = 0; message[i] != '\0'; i++)
    {
        video[cursor++] = (unsigned short)message[i] | 0x0700;
    }

    cursor++;

    /* Keyboard loop */
    while (1)
    {
        char key = keyboard_get_char();

        if (key != 0)
        {
            video[cursor++] = (unsigned short)key | 0x0700;

            if (cursor >= 2000)
            {
                cursor = 0;
            }
        }
    }
}
