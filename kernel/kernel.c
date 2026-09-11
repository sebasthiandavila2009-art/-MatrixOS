// MatrixOS Kernel
// Version 1.0 - First GUI Interaction

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

#define BUTTON_X      110
#define BUTTON_Y      70
#define BUTTON_WIDTH  100
#define BUTTON_HEIGHT 60

int cursor_x = 160;
int cursor_y = 100;

int button_pressed = 0;

/*
 * Draw the main MatrixOS screen.
 */
void draw_screen(void)
{
    graphics_clear(1);

    /*
     * Draw GUI button.
     */
    if (button_pressed)
    {
        /*
         * Button becomes green when clicked.
         */
        graphics_rectangle(
            BUTTON_X,
            BUTTON_Y,
            BUTTON_WIDTH,
            BUTTON_HEIGHT,
            10
        );
    }
    else
    {
        /*
         * Normal white button.
         */
        graphics_rectangle(
            BUTTON_X,
            BUTTON_Y,
            BUTTON_WIDTH,
            BUTTON_HEIGHT,
            15
        );
    }
}

/*
 * Draw the MatrixOS mouse cursor.
 */
void draw_cursor(int x, int y)
{
    /*
     * Black cursor outline.
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
     * White cursor interior.
     */
    graphics_rectangle(x + 2, y + 2, 2, 12, 15);
    graphics_rectangle(x + 4, y + 4, 2, 11, 15);
    graphics_rectangle(x + 6, y + 6, 2, 10, 15);
    graphics_rectangle(x + 8, y + 8, 2, 9, 15);
    graphics_rectangle(x + 10, y + 10, 2, 7, 15);
    graphics_rectangle(x + 12, y + 12, 2, 6, 15);
}

/*
 * Check whether the cursor is inside the button.
 */
int cursor_over_button(void)
{
    if (cursor_x >= BUTTON_X &&
        cursor_x < BUTTON_X + BUTTON_WIDTH &&
        cursor_y >= BUTTON_Y &&
        cursor_y < BUTTON_Y + BUTTON_HEIGHT)
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

    draw_screen();
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
             * Left mouse button pressed.
             */
            if (buttons & 0x01)
            {
                if (cursor_over_button())
                {
                    button_pressed = 1;
                }
            }

            /*
             * Redraw screen and cursor.
             */
            draw_screen();
            draw_cursor(cursor_x, cursor_y);
        }
    }
}
