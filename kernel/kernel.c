// MatrixOS Kernel
// Version 2.7 - Smoother Desktop + Terminal Controls

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define BLUE 1
#define WHITE 15
#define BLACK 0
#define LIGHT_BLUE 9
#define GREEN 10
#define RED 4
#define GRAY 8

extern void graphics_put_pixel(int x, int y, unsigned char color);
extern void graphics_clear(unsigned char color);
extern void graphics_rectangle(
    int x,
    int y,
    int width,
    int height,
    unsigned char color
);

extern char keyboard_get_char(void);

extern void mouse_init(void);
extern int mouse_get_packet(
    int *dx,
    int *dy,
    unsigned char *buttons
);

/* Compact 5x7 uppercase font. */
static const unsigned char font[26][7] = {
    {14,17,17,31,17,17,17},
    {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14},
    {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31},
    {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,14},
    {17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14},
    {7,2,2,2,2,18,12},
    {17,18,20,24,20,18,17},
    {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17},
    {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14},
    {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13},
    {30,17,17,30,20,18,17},
    {14,17,16,14,1,17,14},
    {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14},
    {17,17,17,17,17,10,4},
    {17,17,17,21,21,27,17},
    {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4},
    {31,1,2,4,8,16,31}
};

static const unsigned char digits[10][7] = {
    {14,17,19,21,25,17,14},
    {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31},
    {31,2,4,2,1,17,14},
    {2,6,10,18,31,2,2},
    {31,16,30,1,1,17,14},
    {6,8,16,30,17,17,14},
    {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14},
    {14,17,17,15,1,2,12}
};

static void draw_char(
    int x,
    int y,
    char c,
    unsigned char color
)
{
    const unsigned char *g = 0;

    if (c >= 'a' && c <= 'z')
        c = (char)(c - 'a' + 'A');

    if (c >= 'A' && c <= 'Z')
        g = font[c - 'A'];
    else if (c >= '0' && c <= '9')
        g = digits[c - '0'];

    if (g)
    {
        for (int row = 0; row < 7; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                if (g[row] & (1 << (4 - col)))
                {
                    graphics_put_pixel(
                        x + col,
                        y + row,
                        color
                    );
                }
            }
        }
    }
    else if (c == '-')
    {
        graphics_rectangle(
            x,
            y + 3,
            5,
            1,
            color
        );
    }
    else if (c == '_')
    {
        graphics_rectangle(
            x,
            y + 6,
            5,
            1,
            color
        );
    }
    else if (c == '>')
    {
        graphics_put_pixel(x, y + 3, color);
        graphics_put_pixel(x + 1, y + 2, color);
        graphics_put_pixel(x + 2, y + 1, color);
        graphics_put_pixel(x + 1, y + 4, color);
        graphics_put_pixel(x + 2, y + 5, color);
    }
    else if (c == ':')
    {
        graphics_rectangle(
            x + 2,
            y + 2,
            2,
            2,
            color
        );

        graphics_rectangle(
            x + 2,
            y + 5,
            2,
            2,
            color
        );
    }
    else if (c == '.')
    {
        graphics_rectangle(
            x + 2,
            y + 6,
            2,
            1,
            color
        );
    }
}

static void draw_text(
    int x,
    int y,
    const char *text,
    unsigned char color
)
{
    while (*text)
    {
        draw_char(x, y, *text, color);
        x += 6;
        text++;
    }
}

static int string_equal(
    const char *a,
    const char *b
)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;

        a++;
        b++;
    }

    return *a == 0 && *b == 0;
}


/* =========================
   DESKTOP STATE
   ========================= */

static int terminal_open = 0;

static char terminal_input[64];
static int terminal_length = 0;

static int terminal_x = 25;
static int terminal_y = 25;

static int terminal_dragging = 0;

static int terminal_drag_offset_x = 0;
static int terminal_drag_offset_y = 0;


/* =========================
   TERMINAL OUTPUT
   ========================= */

static char terminal_output[5][43];

static void clear_output(void)
{
    for (int row = 0; row < 5; row++)
    {
        terminal_output[row][0] = 0;
    }
}

static void set_output(
    int row,
    const char *text
)
{
    if (row < 0 || row >= 5)
        return;

    int i = 0;

    while (text[i] && i < 42)
    {
        terminal_output[row][i] = text[i];
        i++;
    }

    terminal_output[row][i] = 0;
}


/* =========================
   MOUSE STATE
   ========================= */

static int mouse_x = 160;
static int mouse_y = 100;

static unsigned char mouse_buttons = 0;


/*
 * Save the pixels underneath the cursor.
 *
 * This lets us move the cursor without
 * redrawing the entire screen every time.
 */
static unsigned char cursor_background[9][9];

static int cursor_saved = 0;

static void save_cursor_background(void)
{
    for (int y = 0; y < 9; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            /*
             * We cannot directly read the framebuffer
             * through the graphics driver, so this
             * function is intentionally left empty.
             *
             * The cursor is instead restored by redrawing
             * the affected desktop/window area.
             */
        }
    }

    cursor_saved = 1;
}


/* =========================
   DESKTOP ICONS
   ========================= */

static void draw_terminal_icon(
    int x,
    int y
)
{
    graphics_rectangle(
        x,
        y,
        42,
        32,
        BLACK
    );

    graphics_rectangle(
        x + 2,
        y + 2,
        38,
        28,
        WHITE
    );

    draw_char(
        x + 7,
        y + 9,
        '>',
        BLACK
    );

    draw_char(
        x + 14,
        y + 9,
        '_',
        BLACK
    );
}

static void draw_files_icon(
    int x,
    int y
)
{
    graphics_rectangle(
        x,
        y + 5,
        42,
        27,
        WHITE
    );

    graphics_rectangle(
        x + 5,
        y,
        19,
        8,
        WHITE
    );
}

static void draw_settings_icon(
    int x,
    int y
)
{
    graphics_rectangle(
        x + 5,
        y + 5,
        32,
        22,
        GRAY
    );

    graphics_rectangle(
        x + 13,
        y + 9,
        16,
        14,
        BLACK
    );
}

static void draw_about_icon(
    int x,
    int y
)
{
    graphics_rectangle(
        x,
        y,
        42,
        32,
        LIGHT_BLUE
    );

    draw_char(
        x + 18,
        y + 5,
        'I',
        WHITE
    );
}


/* =========================
   TERMINAL WINDOW
   ========================= */

static void draw_close_button(void)
{
    int center_x = terminal_x + 258;
    int center_y = terminal_y + 8;

    /*
     * Red circular close button.
     */
    for (int y = -4; y <= 4; y++)
    {
        for (int x = -4; x <= 4; x++)
        {
            if ((x * x) + (y * y) <= 16)
            {
                graphics_put_pixel(
                    center_x + x,
                    center_y + y,
                    RED
                );
            }
        }
    }
}

static void draw_terminal_window(void)
{
    if (!terminal_open)
        return;

    int x = terminal_x;
    int y = terminal_y;

    /* Shadow. */
    graphics_rectangle(
        x + 3,
        y + 3,
        270,
        140,
        BLACK
    );

    /* Window border. */
    graphics_rectangle(
        x,
        y,
        270,
        140,
        WHITE
    );

    /* Title bar. */
    graphics_rectangle(
        x,
        y,
        270,
        16,
        BLACK
    );

    draw_text(
        x + 8,
        y + 5,
        "MATRIX TERMINAL",
        WHITE
    );

    /* Red close button. */
    draw_close_button();

    /* Window body. */
    graphics_rectangle(
        x,
        y + 20,
        270,
        120,
        BLACK
    );

    draw_text(
        x + 12,
        y + 27,
        "MATRIXOS TERMINAL",
        GREEN
    );

    draw_text(
        x + 12,
        y + 39,
        "TYPE HELP FOR COMMANDS",
        WHITE
    );

    /*
     * Persistent command output.
     */
    for (int row = 0; row < 5; row++)
    {
        if (terminal_output[row][0])
        {
            draw_text(
                x + 12,
                y + 51 + row * 9,
                terminal_output[row],
                WHITE
            );
        }
    }

    /*
     * Command prompt.
     */
    draw_text(
        x + 12,
        y + 102,
        "MATRIXBOOK $",
        GREEN
    );

    draw_text(
        x + 90,
        y + 102,
        terminal_input,
        WHITE
    );

    /*
     * Cursor.
     */
    graphics_rectangle(
        x + 90 + terminal_length * 6,
        y + 101,
        5,
        8,
        WHITE
    );
}


/* =========================
   DESKTOP
   ========================= */

static void draw_desktop(void)
{
    graphics_clear(BLUE);

    /* Top bar. */
    graphics_rectangle(
        0,
        0,
        SCREEN_WIDTH,
        18,
        BLACK
    );

    draw_text(
        8,
        5,
        "MATRIXBOOK",
        WHITE
    );

    draw_text(
        250,
        5,
        "MATRIXOS",
        WHITE
    );

    /* Icons. */
    draw_terminal_icon(20, 35);
    draw_files_icon(100, 35);
    draw_settings_icon(180, 35);
    draw_about_icon(260, 35);

    draw_text(
        20,
        72,
        "TERMINAL",
        WHITE
    );

    draw_text(
        104,
        72,
        "FILES",
        WHITE
    );

    draw_text(
        181,
        72,
        "SETTINGS",
        WHITE
    );

    draw_text(
        266,
        72,
        "ABOUT",
        WHITE
    );

    /* Bottom dock. */
    graphics_rectangle(
        45,
        170,
        230,
        25,
        BLACK
    );

    draw_text(
        58,
        179,
        "TERMINAL",
        WHITE
    );

    draw_terminal_window();
}


/* =========================
   MOUSE CURSOR
   ========================= */

static void draw_cursor(void)
{
    for (int i = 0; i < 9; i++)
    {
        graphics_put_pixel(
            mouse_x,
            mouse_y + i,
            WHITE
        );

        if (i < 6)
        {
            graphics_put_pixel(
                mouse_x + i,
                mouse_y + i,
                WHITE
            );
        }
    }
}


/*
 * Redraw the whole desktop.
 *
 * This is now only used when something
 * actually changes visually.
 */
static void redraw_all(void)
{
    draw_desktop();
    draw_cursor();
}


/* =========================
   MOUSE HANDLING
   ========================= */

static void handle_mouse(void)
{
    int dx;
    int dy;

    unsigned char buttons;

    if (!mouse_get_packet(
            &dx,
            &dy,
            &buttons))
    {
        return;
    }

    /*
     * If the window is not being dragged,
     * we redraw the desktop first.
     *
     * This prevents the old cursor from
     * remaining on screen.
     */
    if (!terminal_dragging)
    {
        draw_desktop();
    }

    mouse_x += dx;
    mouse_y -= dy;

    /*
     * Keep cursor inside screen.
     */
    if (mouse_x < 0)
        mouse_x = 0;

    if (mouse_x > SCREEN_WIDTH - 9)
        mouse_x = SCREEN_WIDTH - 9;

    if (mouse_y < 18)
        mouse_y = 18;

    if (mouse_y > SCREEN_HEIGHT - 9)
        mouse_y = SCREEN_HEIGHT - 9;


    /*
     * Left button JUST pressed.
     */
    if ((buttons & 1) &&
        !(mouse_buttons & 1))
    {
        /*
         * Terminal icon.
         */
        if (mouse_x >= 20 &&
            mouse_x < 62 &&
            mouse_y >= 35 &&
            mouse_y < 67)
        {
            terminal_open = 1;

            terminal_length = 0;
            terminal_input[0] = 0;

            clear_output();

            redraw_all();
        }

        /*
         * Red close button.
         */
        else if (terminal_open &&
                 mouse_x >= terminal_x + 252 &&
                 mouse_x <= terminal_x + 266 &&
                 mouse_y >= terminal_y + 2 &&
                 mouse_y <= terminal_y + 14)
        {
            terminal_open = 0;

            terminal_dragging = 0;

            redraw_all();
        }

        /*
         * Terminal title bar.
         */
        else if (terminal_open &&
                 mouse_x >= terminal_x &&
                 mouse_x < terminal_x + 270 &&
                 mouse_y >= terminal_y &&
                 mouse_y < terminal
