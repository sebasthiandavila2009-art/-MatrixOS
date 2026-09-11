// MatrixOS Kernel
// Version 0.8 - Mouse Cursor

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

int cursor_x = 160;
int cursor_y = 100;

/*
 * Draw the MatrixOS test screen.
 */
void draw_screen(void)
{
    graphics_clear(1);

    graphics_rectangle(
        110,
        70,
        100,
        60,
        15
    );
}

/*
 * Draw a simple mouse cursor.
 */
void draw_cursor(int x, int y)
{
    /*
     * Black cursor outline.
     */
    graphics_rectangle(x, y, 3, 12, 0);
    graphics_rectangle(x, y, 10, 3, 0);

    /*
     * White cursor body.
     */
    graphics_rectangle(x + 2, y + 2, 2, 7, 15);
    graphics_rectangle(x + 2, y + 2, 6, 2, 15);
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

    draw_screen();
    draw_cursor(cursor_x, cursor_y);

    while (1)
    {
        if (mouse_get_packet(&dx, &dy, &buttons))
        {
            /*
             * Mouse Y movement is inverted:
             * moving the mouse up produces positive Y data.
             */
            cursor_x += dx;
            cursor_y -= dy;

            /*
             * Keep cursor on screen.
             */
            if (cursor_x < 0)
                cursor_x = 0;

            if (cursor_x > SCREEN_WIDTH - 10)
                cursor_x = SCREEN_WIDTH - 10;

            if (cursor_y < 0)
                cursor_y = 0;

            if (cursor_y > SCREEN_HEIGHT - 12)
                cursor_y = SCREEN_HEIGHT - 12;

            /*
             * Redraw the screen and cursor.
             */
            draw_screen();
            draw_cursor(cursor_x, cursor_y);

            (void)buttons;
        }
    }
}
