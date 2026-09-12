// MatrixOS Kernel
// Version 2.1 - MatrixBook Desktop

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

#define TEXT_START_X     50
#define TEXT_START_Y     60

#define CURSOR_WIDTH     20
#define CURSOR_HEIGHT    22

#define MAX_TEXT         512

int cursor_x = 160;
int cursor_y = 100;

int terminal_open = 0;

int text_x = TEXT_START_X;
int text_y = TEXT_START_Y;

static char terminal_text[MAX_TEXT];
int terminal_text_length = 0;

static unsigned char cursor_background[
    CURSOR_WIDTH * CURSOR_HEIGHT
];

static int cursor_visible = 0;

volatile unsigned char *video_memory =
    (volatile unsigned char *)0xA0000;


/*
 * 5x7 font.
 */

static const unsigned char font[26][7] =
{
    {14,17,17,31,17,17,17},
    {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14},
    {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31},
    {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15},
    {17,17,17,31,17,17,17},
    {31,4,4,4,4,4,31},
    {7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17},
    {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17},
    {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14},
    {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13},
    {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30},
    {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14},
    {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10},
    {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4},
    {31,1,2,4,8,16,31}
};


/*
 * Get font row.
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
 * Draw character with selected color.
 */

void draw_character_at(
    char c,
    int x,
    int y,
    unsigned char color)
{
    int row;
    int col;

    if (c >= 'a' && c <= 'z')
        c -= 32;

    if (c < 'A' || c > 'Z')
        return;

    for (row = 0; row < 7; row++)
    {
        unsigned char pattern =
            get_font_row(c, row);

        for (col = 0; col < 5; col++)
        {
            if (pattern & (1 << (4 - col)))
            {
                graphics_rectangle(
                    x + col,
                    y + row,
                    1,
                    1,
                    color
                );
            }
        }
    }
}


/*
 * Draw text with selected color.
 */

void draw_text(
    const char *text,
    int x,
    int y,
    unsigned char color)
{
    while (*text)
    {
        if (*text == ' ')
        {
            x += 6;
        }
        else
        {
            draw_character_at(
                *text,
                x,
                y,
                color
            );

            x += 6;
        }

        text++;
    }
}


/*
 * Draw terminal text.
 */

void draw_terminal_text(void)
{
    int i;

    int x = TEXT_START_X;
    int y = TEXT_START_Y;

    for (i = 0; i < terminal_text_length; i++)
    {
        char c = terminal_text[i];

        if (c == '\n')
        {
            x = TEXT_START_X;
            y += 10;
            continue;
        }

        if (c == ' ')
        {
            x += 6;
            continue;
        }

        draw_character_at(
            c,
            x,
            y,
            15
        );

        x += 6;

        if (x > 265)
        {
            x = TEXT_START_X;
            y += 10;
        }

        if (y > 145)
            break;
    }

    text_x = x;
    text_y = y;
}


/*
 * Draw MatrixBook desktop.
 */

void draw_desktop(void)
{
    /*
     * Main desktop.
     */

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

    draw_text(
        "MATRIXOS",
        8,
        5,
        15
    );

    draw_text(
        "MATRIXBOOK",
        252,
        5,
        15
    );


    /*
     * Terminal application.
     */

    graphics_rectangle(
        20,
        38,
        50,
        40,
        15
    );

    graphics_rectangle(
        28,
        48,
        34,
        4,
        0
    );

    draw_text(
        "TERMINAL",
        20,
        84,
        15
    );


    /*
     * Files application.
     */

    graphics_rectangle(
        92,
        38,
        50,
        40,
        15
    );

    graphics_rectangle(
        100,
        48,
        34,
        4,
        0
    );

    draw_text(
        "FILES",
        101,
        84,
        15
    );


    /*
     * Settings application.
     */

    graphics_rectangle(
        164,
        38,
        50,
        40,
        15
    );

    graphics_rectangle(
        172,
        48,
        34,
        4,
        0
    );

    draw_text(
        "SETTINGS",
        165,
        84,
        15
    );


    /*
     * About application.
     */

    graphics_rectangle(
        236,
        38,
        50,
        40,
        15
    );

    graphics_rectangle(
        244,
        48,
        34,
        4,
        0
    );

    draw_text(
        "ABOUT",
        245,
        84,
        15
    );


    /*
     * Bottom dock.
     */

    graphics_rectangle(
        0,
        SCREEN_HEIGHT - 18,
        SCREEN_WIDTH,
        18,
        0
    );

    /*
     * Dock items.
     */

    graphics_rectangle(
        110,
        188,
        12,
        8,
        15
    );

    graphics_rectangle(
        130,
        188,
        12,
        8,
        15
    );

    graphics_rectangle(
        150,
        188,
        12,
        8,
        15
    );

    graphics_rectangle(
        170,
        188,
        12,
        8,
        15
    );
}


/*
 * Draw Terminal window.
 */

void draw_terminal(void)
{
    /*
     * Window.
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
     * Terminal title.
     * Black text on white.
     */

    draw_text(
        "TERMINAL",
        TERMINAL_X + 8,
        TERMINAL_Y + 5,
        0
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
     * Terminal display.
     */

    graphics_rectangle(
        45,
        50,
        230,
        105,
        0
    );

    /*
     * Terminal text.
     */

    draw_terminal_text();

    /*
     * Terminal cursor.
     */

    graphics_rectangle(
        text_x,
        text_y,
        2,
        7,
        10
    );
}


/*
 * Save pixels underneath cursor.
 */

void cursor_save_background(void)
{
    int x;
    int y;

    for (y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = cursor_x + x;
            int py = cursor_y + y;

            if (px >= 0 &&
                px < SCREEN_WIDTH &&
                py >= 0 &&
                py < SCREEN_HEIGHT)
            {
                cursor_background[
                    y * CURSOR_WIDTH + x
                ] =
                    video_memory[
                        py * SCREEN_WIDTH + px
                    ];
            }
            else
            {
                cursor_background[
                    y * CURSOR_WIDTH + x
                ] = 0;
            }
        }
    }
}


/*
 * Restore pixels underneath cursor.
 */

void cursor_restore_background(void)
{
    int x;
    int y;

    if (!cursor_visible)
        return;

    for (y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = cursor_x + x;
            int py = cursor_y + y;

            if (px >= 0 &&
                px < SCREEN_WIDTH &&
                py >= 0 &&
                py < SCREEN_HEIGHT)
            {
                video_memory[
                    py * SCREEN_WIDTH + px
                ] =
                    cursor_background[
                        y * CURSOR_WIDTH + x
                    ];
            }
        }
    }

    cursor_visible = 0;
}


/*
 * Draw MatrixOS cursor.
 */

void draw_cursor(void)
{
    int x = cursor_x;
    int y = cursor_y;

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

    cursor_visible = 1;
}


/*
 * Erase cursor.
 */

void cursor_erase(void)
{
    cursor_restore_background();
}


/*
 * Show cursor.
 */

void cursor_show(void)
{
    if (cursor_visible)
        return;

    cursor_save_background();
    draw_cursor();
}


/*
 * Redraw entire screen.
 */

void redraw_screen(void)
{
    cursor_erase();

    draw_desktop();

    if (terminal_open)
    {
        draw_terminal();
    }

    cursor_show();
}


/*
 * Is cursor over Terminal?
 */

int cursor_over_terminal(void)
{
    if (cursor_x >= 20 &&
        cursor_x < 70 &&
        cursor_y >= 38 &&
        cursor_y < 78)
    {
        return 1;
    }

    return 0;
}


/*
 * Is cursor over Terminal close?
 */

int cursor_over_close(void)
{
    if (cursor_x >= 268 &&
        cursor_x < 278 &&
        cursor_y >= 29 &&
        cursor_y < 39)
    {
        return 1;
    }

    return 0;
}


/*
 * Open Terminal.
 */

void open_terminal(void)
{
    terminal_open = 1;

    redraw_screen();
}


/*
 * Close Terminal.
 */

void close_terminal(void)
{
    terminal_open = 0;

    redraw_screen();
}


/*
 * Add terminal character.
 */

void add_terminal_character(char key)
{
    if (terminal_text_length >= MAX_TEXT - 1)
        return;

    terminal_text[
        terminal_text_length
    ] = key;

    terminal_text_length++;
}


/*
 * Keyboard handler.
 */

void handle_keyboard(void)
{
    char key;

    key = keyboard_get_char();

    if (key == 0)
        return;

    if (key == '\b')
    {
        if (terminal_text_length > 0)
        {
            terminal_text_length--;

            redraw_screen();
        }

        return;
    }

    if (key == '\n')
    {
        add_terminal_character('\n');

        redraw_screen();

        return;
    }

    if ((key >= 'a' && key <= 'z') ||
        (key >= 'A' && key <= 'Z') ||
        key == ' ')
    {
        add_terminal_character(key);

        redraw_screen();
    }
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

    cursor_show();

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
            int new_left_click =
                ((buttons & 1) &&
                 !(old_buttons & 1));

            cursor_erase();

            cursor_x += dx;
            cursor_y -= dy;

            /*
             * Keep cursor inside screen.
             */

            if (cursor_x < 0)
                cursor_x = 0;

            if (cursor_x >
                SCREEN_WIDTH - CURSOR_WIDTH)
            {
                cursor_x =
                    SCREEN_WIDTH - CURSOR_WIDTH;
            }

            if (cursor_y < 0)
                cursor_y = 0;

            if (cursor_y >
                SCREEN_HEIGHT - CURSOR_HEIGHT)
            {
                cursor_y =
                    SCREEN_HEIGHT - CURSOR_HEIGHT;
            }

            /*
             * Handle clicks.
             */

            if (new_left_click)
            {
                if (!terminal_open &&
                    cursor_over_terminal())
                {
                    open_terminal();
                }
                else if (terminal_open &&
                         cursor_over_close())
                {
                    close_terminal();
                }
            }

            old_buttons = buttons;

            cursor_show();
        }
    }
}
