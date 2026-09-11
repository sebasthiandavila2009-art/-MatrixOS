// MatrixOS Kernel
// Version 0.7 - Graphics Test

extern char keyboard_get_char(void);
extern void mouse_init(void);

extern void graphics_clear(unsigned char color);
extern void graphics_rectangle(
    int x,
    int y,
    int width,
    int height,
    unsigned char color
);

void kernel_main(void)
{
    /*
     * Clear the graphics screen.
     * Color 1 = blue.
     */
    graphics_clear(1);

    /*
     * Draw a white rectangle in the center.
     * Color 15 = white.
     */
    graphics_rectangle(
        110,
        70,
        100,
        60,
        15
    );

    /*
     * Initialize the mouse.
     */
    mouse_init();

    /*
     * Keep MatrixOS running.
     */
    while (1)
    {
        char key = keyboard_get_char();

        /*
         * Keyboard is still active.
         * We don't display keys yet because
         * we're testing graphics first.
         */
        (void)key;
    }
}
