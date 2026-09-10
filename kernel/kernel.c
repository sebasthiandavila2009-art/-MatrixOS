// MatrixOS Kernel
// Version 0.2

void kernel_main(void)
{
    volatile unsigned short *video = (unsigned short *)0xB8000;

    const char *message = "MATRIXOS v0.2 - Kernel Started";

    for (int i = 0; message[i] != '\0'; i++)
    {
        video[i] = (unsigned short)message[i] | 0x0700;
    }

    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
