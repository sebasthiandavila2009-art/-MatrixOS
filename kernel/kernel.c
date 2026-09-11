// MatrixOS Kernel
// Version 1.2 - Terminal Window

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

#define TERMINAL_X       20
#define TERMINAL_Y       40
#define TERMINAL_WIDTH   50
#define TERMINAL_HEIGHT  40

int cursor_x = 160;
int cursor_y = 100;

int terminal_open = 0;

/*
 * Draw the MatrixOS desktop.
 */
void draw_desktop(void)
{
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
     * Icon details.
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
 * Draw Terminal window.
 */
void draw_terminal(void)
{
    /*
     * Window body.
     */
    graphics_rectangle(
        35,
        25,
        250,
        140,
        0
    );

    /*
     * Window title bar.
     */
    graphics_rectangle(
        35,
        25,
        250,
        18,
        15
    );

    /*
     * Close button.
     */
    graphics_rectangle(
        268,
        29,
        10,
        10,
        4
    );

    /*
     * Terminal area.
     */
    graphics_rectangle(
        45,
        50,
        230,
        105,
        0
    );

    /*
     * Simple prompt.
     */
    graphics_rectangle(
        50,
        60,
        4,
        8,
        10
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
 * Check if cursor is over Terminal icon.
 */
int cursor_over_terminal(void)
{
    if (cursor_x >= 20 &&
        cursor_x < 70 &&
        cursor_y >= 40 &&
        cursor_y < 80)
    {
        return 1;
    }

    return 0;
}

/*
 * MatrixOS kernel entry point.
 */
void kernel_main(void)
{
    int dx;
    int dy;
    unsigned char buttons;

    mouse_init();

    draw_desktop();
    draw_cursor(cursor_x, cursor_y);

    while (1)
    {
        if (mouse_get_packet(&dx, &dy, &buttons))
        {
            /*
             * Update cursor position.
             */
            cursor_x += dx;
            cursor_y -= dy;

            /*
             * Keep cursor on screen.
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
             * Left-click Terminal.
             */
            if ((buttons & 0x01) &&
                cursor_over_terminal())
            {
                terminal_open = 1;
            }

            /*
             * Redraw desktop.
             */
            draw_desktop();

            /*
             * Draw Terminal if opened.
             */
            if (terminal_open)
            {
                draw_terminal();
            }

            /*
             * Always draw cursor last.
             */
            draw_cursor(cursor_x, cursor_y);
        }
    }
}
