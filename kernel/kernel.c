// MatrixOS Kernel
// Version 1.4 - Terminal Text Renderer

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

#define FONT_SCALE 1
#define FONT_WIDTH 5
#define FONT_HEIGHT 7

int cursor_x = 160;
int cursor_y = 100;

int terminal_open = 0;

int text_x = 50;
int text_y = 60;

/*
 * Simple 5x7 font.
 */
unsigned char font_char(char c, int row)
{
    static const unsigned char font[][7] =
    {
        /* A */
        {14,17,17,31,17,17,17},

        /* B */
        {30,17,17,30,17,17,30},

        /* C */
        {14,17,16,16,16,17,14},

        /* D */
        {30,17,17,17,17,17,30},

        /* E */
        {31,16,16,30,16,16,31},

        /* F */
        {31,16,16,30,16,16,16},

        /* G */
        {14,17,16,23,17,17,15},

        /* H */
        {17,17,17,31,17,17,17},

        /* I */
        {31,4,4,4,4,4,31},

        /* J */
        {7,2,2,2,18,18,12},

        /* K */
        {17,18,20,24,20,18,17},

        /* L */
        {16,16,16,16,16,16,31},

        /* M */
        {17,27,21,21,17,17,17},

        /* N */
        {17,25,21,19,17,17,17},

        /* O */
        {14,17,17,17,17,17,14},

        /* P */
        {30,17,17,30,16,16,16},

        /* Q */
        {14,17,17,17,21,18,13},

        /* R */
        {30,17,17,30,20,18,17},

        /* S */
        {15,16,16,14,1,1,30},

        /* T */
        {31,4,4,4,4,4,4},

        /* U */
        {17,17,17,17,17,17,14},

        /* V */
        {17,17,17,17,17,10,4},

        /* W */
        {17,17,17,21,21,21,10},

        /* X */
        {17,17,10,4,10,17,17},

        /* Y */
        {17,17,10,4,4,4,4},

        /* Z */
        {31,1,2,4,8,16,31}
    };

    if (c >= 'a' && c <= 'z')
        c -= 32;

    if (c < 'A' || c > 'Z')
        return 0;

    return font_char_data(font, c - 'A', row);
}

/*
 * Helper for font lookup.
 */
unsigned char font_char_data(
    const unsigned char font[][7],
    int index,
    int row
)
{
    return font[index][row];
}

/*
 * Draw a character.
 */
void draw_character(char c)
{
    int row;
    int col;

    if (c == ' ')
    {
        text_x += 6;
        return;
    }

    if (c < 'A' || c > 'Z')
    {
        if (c >= 'a' && c <= 'z')
            c -= 32;
        else
            return;
    }

    /*
     * Use a compact built-in pattern.
     */
    for (row = 0; row < 7; row++)
    {
        unsigned char pattern;

        pattern = font_char(c, row);

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
 * Draw desktop.
 */
void draw_desktop(void)
{
    graphics_clear(1);

    graphics_rectangle(
        0, 0,
        SCREEN_WIDTH, 18,
        0
    );

    graphics_rectangle(
        0,
        SCREEN_HEIGHT - 16,
        SCREEN_WIDTH,
        16,
        0
    );

    /* Terminal */
    graphics_rectangle(
        20, 40,
        50, 40,
        15
    );

    /* Files */
    graphics_rectangle(
        90, 40,
        50, 40,
        15
    );

    /* Settings */
    graphics_rectangle(
        160, 40,
        50, 40,
        15
    );

    graphics_rectangle(
        28, 50,
        34, 4,
        0
    );

    graphics_rectangle(
        98, 50,
        34, 4,
        0
    );

    graphics_rectangle(
        168, 50,
        34, 4,
        0
    );
}

/*
 * Draw terminal window.
 */
void draw_terminal(void)
{
    graphics_rectangle(
        TERMINAL_X,
        TERMINAL_Y,
        TERMINAL_WIDTH,
        TERMINAL_HEIGHT,
        0
    );

    graphics_rectangle(
        TERMINAL_X,
        TERMINAL_Y,
        TERMINAL_WIDTH,
        18,
        15
    );

    graphics_rectangle(
        268, 29,
        10, 10,
        4
    );

    graphics_rectangle(
        45, 50,
        230, 105,
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
 * Cursor.
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
        x + 2, y + 2,
        2, 12,
        15
    );

    graphics_rectangle(
        x + 4, y + 4,
        2, 11,
        15
    );

    graphics_rectangle(
        x + 6, y + 6,
        2, 10,
        15
    );

    graphics_rectangle(
        x + 8, y + 8,
        2, 9,
        15
    );

    graphics_rectangle(
        x + 10, y + 10,
        2, 7,
        15
    );

    graphics_rectangle(
        x + 12, y + 12,
        2, 6,
        15
    );
}

/*
 * Terminal icon hit detection.
 */
int cursor_over_terminal(void)
{
    return (
        cursor_x >= 20 &&
        cursor_x < 70 &&
        cursor_y >= 40 &&
        cursor_y < 80
    );
}

/*
 * Process keyboard.
 */
void handle_keyboard(void)
{
    char key;

    key = keyboard_get_char();

    if (key == 0)
        return;

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

    if (key == '\n')
    {
        text_x = 50;
        text_y += 10;

        if (text_y > 145)
            text_y = 60;

        return;
    }

    draw_character(key);
}

/*
 * Kernel entry.
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
         * Keyboard is now non-blocking.
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
             * Detect new left click.
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
             * IMPORTANT:
             *
             * Don't redraw the terminal every time
             * the mouse moves, because that would
             * erase typed characters.
             */
            if (!terminal_open)
            {
                draw_desktop();
            }
            else
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
