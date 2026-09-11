// MatrixOS Kernel
// Version 1.1 - MatrixOS Desktop

extern void mouse_init(void);

extern int mouse_get_packet(
    int *dx,
    int *dy,
    unsigned char *buttons
);

extern void graphics_clear(unsigned char color);

extern void graphics_rectangle(
    int x,
    int y,
    int width,
    int height,
    unsigned char color
);

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

#define TOPBAR_HEIGHT 18
#define TASKBAR_HEIGHT 16

int cursor_x = 160;
int cursor_y = 100;

/*
 * Draw a simple MatrixOS desktop.
 */
void draw_desktop(void)
{
    /*
     * Main desktop background.
     */
    graphics_clear(1);

    /*
     * Top bar.
     */
    graphics_rectangle(
        0,
        0,
        SCREEN_WIDTH,
        TOPBAR_HEIGHT,
        0
    );

    /*
     * Taskbar.
     */
    graphics_rectangle(
        0,
        SCREEN_HEIGHT - TASKBAR_HEIGHT,
        SCREEN_WIDTH,
        TASKBAR_HEIGHT,
        0
    );

    /*
     * Terminal icon.
     */
    graphics_rectangle(
        20,
        40,
        50,
        40,
        15
    );

    /*
     * File Manager icon.
     */
    graphics_rectangle(
        90,
        40,
        50,
        40,
        15
    );

    /*
     * Settings icon.
     */
    graphics_rectangle(
        160,
        40,
        50,
        40,
        15
    );

    /*
     * Small icon details.
     */
    graphics_rectangle(
        28,
        50,
        34,
        4,
        0
    );

    graphics_rectangle(
        98,
        50,
        34,
        4,
        0
    );

    graphics_rectangle(
        168,
        50,
        34,
        4,
        0
    );
}

/*
 * Draw the MatrixOS cursor.
 */
void draw_cursor(int x, int y)
{
    /*
     * Black outline.
     */
    graphics_rectangle(x, y, 2, 18, 0);

    graphics_rectangle(x + 2, y + 2, 2, 16, 0);
    graphics_rectangle(x + 4, y + 4, 2, 14, 0);
    graphics_rectangle(x + 6, y + 6, 2, 12, 0);
    graphics_rectangle(x + 8, y + 8, 2, 10, 0);
    graphics_rectangle(x + 10, y + 10, 2, 8, 0);
    graphics_rectangle(x + 12, y + 12, 2, 8, 0);

    graphics_rectangle(x + 14, y + 14, 2, 6, 0);
    graphics_rectangle(x + 16, y + 16, 2, 4, 0);

    graphics_rectangle(x + 8, y + 16, 10, 4, 0);

    /*
     * White interior.
     */
    graphics_rectangle(x + 2, y + 2, 2, 12, 15);
    graphics_rectangle(x + 4, y + 4, 2, 11, 15);
    graphics_rectangle(x + 6, y + 6, 2, 10, 15);
    graphics_rectangle(x + 8, y + 8, 2, 9, 15);
    graphics_rectangle(x + 10, y + 10, 2, 7, 15);
    graphics_rectangle(x + 12, y + 12, 2, 6, 15);
}

/*
 * MatrixOS kernel entry point.
 */
void kernel_main(void)
{
    int dx;
    int dy;
    unsigned char buttons;

    /*
     * Initialize mouse.
     */
    mouse_init();

    /*
     * Draw desktop.
     */
    draw_desktop();

    /*
     * Draw cursor.
     */
    draw_cursor(cursor_x, cursor_y);

    /*
     * Main desktop loop.
     */
    while (1)
    {
        if (mouse_get_packet(&dx, &dy, &buttons))
        {
            /*
             * Update cursor.
             */
            cursor_x += dx;
            cursor_y -= dy;

            /*
             * Keep cursor inside screen.
             */
            if (cursor_x < 0)
                cursor_x = 0;

            if (cursor_x > SCREEN_WIDTH - 18)
                cursor_x = SCREEN_WIDTH - 18;

            if (cursor_y < 0)
                cursor_y = 0;

            if (cursor_y > SCREEN_HEIGHT - 20)
                cursor_y = SCREEN_HEIGHT - 20;

            /*
             * Redraw desktop and cursor.
             */
            draw_desktop();
            draw_cursor(cursor_x, cursor_y);

            (void)buttons;
        }
    }
}
