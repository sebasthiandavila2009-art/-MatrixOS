// MatrixOS Kernel
// Version 1.5 - Stable Terminal Input

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

#define TERMINAL_X 35
#define TERMINAL_Y 25
#define TERMINAL_WIDTH 250
#define TERMINAL_HEIGHT 140

#define TEXT_START_X 50
#define TEXT_START_Y 60

int cursor_x = 160;
int cursor_y = 100;

int terminal_open = 0;

int text_x = TEXT_START_X;
int text_y = TEXT_START_Y;


/*
 * 5x7 font data.
 *
 * Characters supported:
 * A-Z
 */
static const unsigned char font[26][7] =
{
    {14,17,17,31,17,17,17}, /* A */
    {30,17,17,30,17,17,30}, /* B */
    {14,17,16,16,16,17,14}, /* C */
    {30,17,17,17,17,17,30}, /* D */
    {31,16,16,30,16,16,31}, /* E */
    {31,16,16,30,16,16,16}, /* F */
    {14,17,16,23,17,17,15}, /* G */
    {17,17,17,31,17,17,17}, /* H */
    {31,4,4,4,4,4,31},      /* I */
    {7,2,2,2,18,18,12},     /* J */
    {17,18,20,24,20,18,17}, /* K */
    {16,16,16,16,16,16,31}, /* L */
    {17,27,21,21,17,17,17}, /* M */
    {17,25,21,19,17,17,17}, /* N */
    {14,17,17,17,17,17,14}, /* O */
    {30,17,17,30,16,16,16}, /* P */
    {14,17,17,17,21,18,13}, /* Q */
    {30,17,17,30,20,18,17}, /* R */
    {15,16,16,14,1,1,30},    /* S */
    {31,4,4,4,4,4,4},        /* T */
    {17,17,17,17,17,17,14}, /* U */
    {17,17,17,17,17,10,4},  /* V */
    {17,17,17,21,21,21,10}, /* W */
    {17,17,10,4,10,17,17},  /* X */
    {17,17,10,4,4,4,4},     /* Y */
    {31,1,2,4,8,16,31}      /* Z */
};


/*
 * Get font pattern.
 */
unsigned char get_font_row(char c, int row)
{
    if (c >= 'a' && c <= 'z')
        c -= 32;

    if (c < 'A' || c > 'Z')
        return 0;

    if (row < 0 || row >= 7)
        return 0;

    return font[c - 'A'][row];
}


/*
 * Draw a character.
 */
void draw_character(char c)
{
    int row;
    int col;

    /*
     * Space.
     */
    if (c == ' ')
    {
        text_x += 6;
        return;
    }

    /*
     * Convert lowercase to uppercase.
     */
    if (c >= 'a' && c <= 'z')
        c -= 32;

    /*
     * Only draw A-Z for now.
     */
    if (c < 'A' || c > 'Z')
        return;

    for (row = 0; row < 7; row++)
    {
        unsigned char pattern;

        pattern = get_font_row(c, row);

        for (col = 0; col < 5; col++)
        {
            if (pattern & (1 << (4 - col)))
            {
                graphics_rectangle(
                    text_x + col,
                    text_y + row,
                    1,
                    1,
                    15
                );
            }
        }
    }

    text_x += 6;

    /*
     * New line when reaching the right side.
     */
    if (text_x > 265)
    {
        text_x = TEXT_START_X;
        text_y += 10;
    }

    /*
     * Keep text inside the terminal.
     */
    if (text_y > 145)
    {
        text_x = TEXT_START_X;
        text_y = TEXT_START_Y;
    }
}


/*
 * Draw desktop.
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
        18,
        0
    );

    /*
     * Taskbar.
     */
    graphics_rectangle(
        0,
        SCREEN_HEIGHT - 16,
        SCREEN_WIDTH,
        16,
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
     * Files icon.
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
 * Draw terminal window.
 */
void draw_terminal(void)
{
    /*
     * Main window.
     */
    graphics_rectangle(
        TERMINAL_X,
        TERMINAL_Y,
        TERMINAL_WIDTH,
        TERMINAL_HEIGHT,
        0
    );

    /*
     * Title bar.
     */
    graphics_rectangle(
        TERMINAL_X,
        TERMINAL_Y,
        TERMINAL_WIDTH,
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
     * Terminal screen.
     */
    graphics_rectangle(
        45,
        50,
        230,
        105,
        0
    );

    /*
     * Prompt.
     */
    graphics_rectangle(
        50,
        60,
        2,
        7,
        10
    );
}


/*
 * Draw MatrixOS cursor.
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

    graphics_rectangle(
        x + 8,
        y + 16,
        10,
        4,
        0
    );

    /*
     * White interior.
     */
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
 * Is the cursor over Terminal?
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
 * Process keyboard input.
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
        if (text_x > TEXT_START_X)
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
        text_x = TEXT_START_X;
        text_y += 10;

        if (text_y > 145)
        {
            text_y = TEXT_START_Y;
        }

        return;
    }

    /*
     * Normal character.
     */
    draw_character(key);
}


/*
 * MatrixOS kernel.
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
         * Keyboard.
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

            /*
             * Screen boundaries.
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
             * New left click.
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
             * Only redraw the desktop when
             * Terminal is NOT open.
             *
             * This prevents typed characters
             * from disappearing.
             */
            if (!terminal_open)
            {
                draw_desktop();
            }

            /*
             * Cursor always gets drawn last.
             */
            draw_cursor(
                cursor_x,
                cursor_y
            );
        }
    }
}
