// MatrixOS Kernel
// Version 1.8 - Stable Cursor Redraw

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

#define MAX_TEXT 512

int cursor_x = 160;
int cursor_y = 100;

int terminal_open = 0;

int text_x = TEXT_START_X;
int text_y = TEXT_START_Y;

static char terminal_text[MAX_TEXT];

int terminal_text_length = 0;

/*
 * Saved pixels underneath the mouse cursor.
 */

static unsigned char cursor_background[
    CURSOR_WIDTH * CURSOR_HEIGHT
];

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

    graphics_rectangle(
        0,
        0,
        SCREEN_WIDTH,
        18,
        0
    );

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
 * Draw terminal.
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
 * Access VGA framebuffer.
 */

volatile unsigned char *get_framebuffer(void)
{
    return (volatile unsigned char *)0xA0000;
}


/*
 * Save background underneath
 * the mouse cursor.
 */

void cursor_save_background(void)
{
    int x;
    int y;

    volatile unsigned char *fb =
        get_framebuffer();

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
                    fb[
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
 * Restore previous cursor background.
 */

void cursor_restore_background(void)
{
    int x;
    int y;

    volatile unsigned char *fb =
        get_framebuffer();

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
                fb[
                    py * SCREEN_WIDTH + px
                ] =
                    cursor_background[
                        y * CURSOR
