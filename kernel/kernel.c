// MatrixOS Kernel
// Version 1.3 - Terminal Keyboard Input

extern void mouse_init(void);

extern int mouse_get_packet(
    int *dx,
    int *dy,
    unsigned char *buttons
);

extern char keyboard_get_char(void);

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

#define TERMINAL_X       35
#define TERMINAL_Y       25
#define TERMINAL_WIDTH   250
#define TERMINAL_HEIGHT  140

int cursor_x = 160;
int cursor_y = 100;

int terminal_open = 0;

int text_x = 50;
int text_y = 60;

/*
 * Draw MatrixOS desktop.
 */
void draw_desktop(void)
{
    graphics_clear(1);

    /* Top bar */
    graphics_rectangle(
        0,
        0,
        SCREEN_WIDTH,
        18,
        0
    );

    /* Taskbar */
    graphics_rectangle(
        0,
        SCREEN_HEIGHT - 16,
        SCREEN_WIDTH,
        16,
        0
    );

    /* Terminal */
    graphics_rectangle(
        20,
        40,
        50,
        40,
        15
    );

    /* Files */
    graphics_rectangle(
        90,
        40,
        50,
        40,
        15
    );

    /* Settings */
    graphics_rectangle(
        160,
        40,
        50,
        40,
        15
    );

    /* Icon details */
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
    /* Window */
    graphics_rectangle(
        TERMINAL_X,
        TERMINAL_Y,
        TERMINAL_WIDTH,
        TERMINAL_HEIGHT,
        0
    );

    /* Title bar */
    graphics_rectangle(
        TERMINAL_X,
        TERMINAL_Y,
        TERMINAL_WIDTH,
        18,
        15
    );

    /* Close button */
    graphics_rectangle(
        268,
        29,
        10,
        10,
        4
    );

    /* Terminal screen */
    graphics_rectangle(
        45,
        50,
        230,
        105,
        0
    );

    /*
     * Prompt indicator.
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
 * Draw cursor.
 */
void draw_cursor(int x, int y)
{
    graphics_rectangle(x, y, 2, 18, 0);
    graphics_rectangle(x + 2, y + 2, 2, 16, 0);
    graphics_rectangle(x + 4, y + 4, 2, 14, 0);
    graphics_rectangle(x + 6, y + 6, 2, 12, 0);
    graphics_rectangle(x + 8, y + 8, 2, 10, 0);
    graphics_rectangle(x + 10, y + 10, 2, 8, 0);
    graphics_rectangle(x + 12, y + 12, 2, 8, 0);
    graphics_rectangle(x + 14, y + 14, 2, 6, 0);
    graphics_rectangle(x + 16, y + 16, 2, 4, 0);

    graphics_rectangle(
        x + 8,
        y + 16,
        10,
        4,
        0
    );

    graphics_rectangle(
        x + 2,
        y + 2,
        2,
        12,
        15
    );

    graphics_rectangle(
        x + 4,
        y + 4,
        2,
        11,
        15
    );

    graphics_rectangle(
        x + 6,
        y + 6,
        2,
        10,
        15
    );

    graphics_rectangle(
        x + 8,
        y + 8,
        2,
        9,
        15
    );

    graphics_rectangle(
        x + 10,
        y + 10,
        2,
        7,
        15
    );

    graphics_rectangle(
        x + 12,
        y + 12,
        2,
        6,
        15
    );
}

/*
 * Check if cursor is over Terminal.
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
 * Draw one simple character block.
 *
 * This is temporary.
 * A real MatrixOS font renderer comes next.
 */
void draw_character(char c)
{
    int width = 4;
    int height = 7;

    if (c == ' ')
    {
        text_x += 6;
        return;
    }

    /*
     * Simple visible character representation.
     */
    graphics_rectangle(
        text_x,
        text_y,
        width,
        height,
        15
    );

    text_x += 6;

    if (text_x > 265)
    {
        text_x = 50;
        text_y += 10;
    }

    if (text_y > 145)
    {
        text_x = 50;
        text_y = 60;
    }
}

/*
 * Handle keyboard input.
 */
void handle_keyboard(void)
{
    char key;

    key = keyboard_get_char();

    if (key == 0)
        return;

    /*
     * Backspace.
     */
    if (key == '\b')
    {
        if (text_x > 50)
        {
            text_x -= 6;

            graphics_rectangle(
                text_x,
                text_y,
                6,
                8,
                0
            );
        }

        return;
    }

    /*
     * Enter.
     */
    if (key == '\n')
    {
        text_x = 50;
        text_y += 10;

        if (text_y > 145)
        {
            text_y = 60;
        }

        return;
    }

    /*
     * Normal character.
     */
    draw_character(key);
}

/*
 * Main kernel.
 */
void kernel_main(void)
{
    int dx;
    int dy;

    unsigned char buttons;
    unsigned char old_buttons = 0;

    mouse_init();

    draw_desktop();
    draw_cursor(cursor_x, cursor_y);

    while (1)
    {
        /*
         * Keyboard only works inside Terminal.
         */
        if (terminal_open)
        {
            handle_keyboard();
        }

        /*
         * Mouse.
         */
        if (mouse_get_packet(
                &dx,
                &dy,
                &buttons))
        {
            cursor_x += dx;
            cursor_y -= dy;

            if (cursor_x < 0)
                cursor_x = 0;

            if (cursor_x > SCREEN_WIDTH - 18)
                cursor_x = SCREEN_WIDTH - 18;

            if (cursor_y < 0)
                cursor_y = 0;

            if (cursor_y > SCREEN_HEIGHT - 20)
                cursor_y = SCREEN_HEIGHT - 20;

            /*
             * Detect a NEW left click.
             */
            if ((buttons & 1) &&
                !(old_buttons & 1))
            {
                if (cursor_over_terminal())
                {
                    terminal_open = 1;
                }
            }

            old_buttons = buttons;

            /*
             * Redraw everything.
             */
            draw_desktop();

            if (terminal_open)
            {
                draw_terminal();
            }

            draw_cursor(
                cursor_x,
                cursor_y
            );
        }
    }
}
