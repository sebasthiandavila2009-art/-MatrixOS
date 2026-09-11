// MatrixOS Kernel
// Version 0.4

extern char keyboard_get_char(void);

volatile unsigned short *video = (unsigned short *)0xB8000;

int cursor = 0;

void kernel_main(void)
{
    const char *message = "MATRIXOS v0.4 - Shell Ready";

    for (int i = 0; message[i] != '\0'; i++)
    {
        video[cursor++] = (unsigned short)message[i] | 0x0700;
    }

    cursor++;

    while (1)
    {
        char key = keyboard_get_char();

        video[cursor++] = (unsigned short)key | 0x0700;

        if (cursor >= 2000)
        {
            cursor = 0;
        }
    }
}
