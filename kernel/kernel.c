// MatrixOS Kernel
// Version 1.9 - Stable Cursor + Desktop Redraw

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


/*
 * Screen
 */

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200


/*
 * Terminal
 */

#define TERMINAL_X       35
#define TERMINAL_Y       25
#define TERMINAL_WIDTH   250
#define TERMINAL_HEIGHT  140

#define TEXT_START_X     50
#define TEXT_START_Y     60


/*
 * Mouse cursor
 */

#define CURSOR_WIDTH     20
#define CURSOR_HEIGHT    22


/*
 * Terminal text
 */

#define MAX_TEXT 512


/*
 * Mouse position
 */

int cursor_x = 160;
int cursor_y = 100;


/*
 * Terminal state
 */

int terminal_open = 0;


/*
 * Terminal text position
 */

int text_x = TEXT_START_X;
int text_y = TEXT_START_Y;


/*
 * Terminal text buffer
 */

static char terminal_text[MAX_TEXT];

int terminal_text_length = 0;


/*
 * Cursor background.
 */

static unsigned char cursor_background[
    CURSOR_WIDTH * CURSOR_HEIGHT
];


/*
 * Cursor visibility state.
 *
 * 1 = cursor is currently drawn
 * 0 = cursor is erased
 */

static int cursor_visible = 0;


/*
 * VGA framebuffer.
 */

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
 * Draw character.
 */

void draw_character_at(
    char c,
    int x,
    int y)
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
                    15
                );
            }
        }
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
            y
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
     * Bottom taskbar.
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
 * Draw Terminal.
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
     * Draw existing text.
     */

    draw_terminal_text();

    /*
     * Terminal text cursor.
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
 * Save the pixels underneath
 * the mouse cursor.
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
 * Restore the pixels underneath
 * the mouse cursor.
 */

void cursor_restore_background(void)
{
    int x;
    int y;

    /*
     * Do not restore the cursor twice.
     */

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
 * Draw MatrixOS mouse cursor.
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
 * Erase cursor safely.
 */

void cursor_erase(void)
{
    cursor_restore_background();
}


/*
 * Show cursor safely.
 */

void cursor_show(void)
{
    if (cursor_visible)
        return;

    cursor_save_background();
    draw_cursor();
}


/*
 * Redraw the complete MatrixOS screen.
 */

void redraw_screen(void)
{
    /*
     * Erase cursor only if it
     * is currently visible.
     */

    cursor_erase();

    /*
     * Draw the desktop.
     */

    draw_desktop();

    /*
     * Draw Terminal if open.
     */

    if (terminal_open)
    {
        draw_terminal();
    }

    /*
     * Put cursor back on top.
     */

    cursor_show();
}


/*
 * Check Terminal icon.
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
 * Check Terminal close button.
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
 * Add character.
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
 * Handle keyboard.
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
        if (terminal_text_length > 0)
        {
            terminal_text_length--;

            redraw_screen();
        }

        return;
    }

    /*
     * Enter.
     */

    if (key == '\n')
    {
        add_terminal_character('\n');

        redraw_screen();

        return;
    }

    /*
     * Letters and spaces.
     */

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

    /*
     * Initialize mouse.
     */

    mouse_init();

    /*
     * Draw initial desktop.
     */

    draw_desktop();

    /*
     * Draw initial cursor.
     */

    cursor_show();

    /*
     * Main MatrixOS loop.
     */

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
            /*
             * Remember whether this
             * is a new left click.
             */

            int new_left_click =
                ((buttons & 1) &&
                 !(old_buttons & 1));

            /*
             * Remove old cursor.
             */

            cursor_erase();

            /*
             * Move cursor.
             */

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
             * Handle new left click.
             */

            if (new_left_click)
            {
                /*
                 * Open Terminal.
                 */

                if (!terminal_open &&
                    cursor_over_terminal())
                {
                    open_terminal();
                }

                /*
                 * Close Terminal.
                 */

                else if (terminal_open &&
                         cursor_over_close())
                {
                    close_terminal();
                }
            }

            /*
             * Remember current button state.
             */

            old_buttons = buttons;

            /*
             * If the click opened or closed
             * a window, redraw_screen()
             * already restored and displayed
             * the cursor.
             *
             * Otherwise show the cursor here.
             */

            cursor_show();
        }
    }
}
