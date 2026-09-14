// MatrixOS Kernel
// Version 2.7 - Stable Terminal Desktop

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


/* =========================
   FONT
   ========================= */

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
        graphics_rectangle(x, y + 3, 5, 1, color);
    }
    else if (c == '_')
    {
        graphics_rectangle(x, y + 6, 5, 1, color);
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
        graphics_rectangle(x + 2, y + 2, 2, 2, color);
        graphics_rectangle(x + 2, y + 5, 2, 2, color);
    }
    else if (c == '.')
    {
        graphics_rectangle(x + 2, y + 6, 2, 1, color);
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

    return (*a == 0 && *b == 0);
}


/* =========================
   TERMINAL STATE
   ========================= */

static int terminal_open = 0;

static int terminal_x = 25;
static int terminal_y = 25;

static int terminal_dragging = 0;

static int terminal_drag_offset_x = 0;
static int terminal_drag_offset_y = 0;

static char terminal_input[64];
static int terminal_length = 0;


/* Terminal output lines. */
static char output_line_1[43];
static char output_line_2[43];
static char output_line_3[43];


/* =========================
   MOUSE STATE
   ========================= */

static int mouse_x = 160;
static int mouse_y = 100;

static unsigned char mouse_buttons = 0;


/* =========================
   TERMINAL OUTPUT
   ========================= */

static void clear_output(void)
{
    output_line_1[0] = 0;
    output_line_2[0] = 0;
    output_line_3[0] = 0;
}

static void copy_text(
    char *destination,
    const char *source
)
{
    int i = 0;

    while (source[i] && i < 42)
    {
        destination[i] = source[i];
        i++;
    }

    destination[i] = 0;
}

static void terminal_command(void)
{
    clear_output();

    if (string_equal(terminal_input, "help"))
    {
        copy_text(
            output_line_1,
            "COMMANDS:"
        );

        copy_text(
            output_line_2,
            "HELP ABOUT CLEAR EXIT"
        );
    }
    else if (string_equal(terminal_input, "about"))
    {
        copy_text(
            output_line_1,
            "MATRIXOS VERSION 2.7"
        );

        copy_text(
            output_line_2,
            "MATRIXBOOK DESKTOP"
        );
    }
    else if (string_equal(terminal_input, "clear"))
    {
        clear_output();
    }
    else if (string_equal(terminal_input, "exit"))
    {
        terminal_open = 0;
    }
    else if (terminal_length > 0)
    {
        copy_text(
            output_line_1,
            "UNKNOWN COMMAND"
        );

        copy_text(
            output_line_2,
            "TYPE HELP"
        );
    }
}


/* =========================
   ICONS
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
     * Draw a small red circular button.
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
    int x;
    int y;

    if (!terminal_open)
        return;

    x = terminal_x;
    y = terminal_y;

    /* Shadow. */
    graphics_rectangle(
        x + 3,
        y + 3,
        270,
        140,
        BLACK
    );

    /* Border. */
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

    /* Body. */
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

    /* Persistent output. */
    if (output_line_1[0])
    {
        draw_text(
            x + 12,
            y + 51,
            output_line_1,
            WHITE
        );
    }

    if (output_line_2[0])
    {
        draw_text(
            x + 12,
            y + 60,
            output_line_2,
            WHITE
        );
    }

    if (output_line_3[0])
    {
        draw_text(
            x + 12,
            y + 69,
            output_line_3,
            WHITE
        );
    }

    /* Prompt. */
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

    /* Text cursor. */
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

    /* Desktop icons. */
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
   CURSOR
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


/* =========================
   FULL REDRAW
   ========================= */

static void redraw_all(void)
{
    draw_desktop();
    draw_cursor();
}


/* =========================
   MOUSE
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
     * Move cursor.
     */
    mouse_x += dx;
    mouse_y -= dy;

    /* Screen limits. */
    if (mouse_x < 0)
        mouse_x = 0;

    if (mouse_x > SCREEN_WIDTH - 9)
        mouse_x = SCREEN_WIDTH - 9;

    if (mouse_y < 18)
        mouse_y = 18;

    if (mouse_y > SCREEN_HEIGHT - 9)
        mouse_y = SCREEN_HEIGHT - 9;


    /*
     * Left button pressed.
     */
    if ((buttons & 1) &&
        !(mouse_buttons & 1))
    {
        /*
         * Open Terminal.
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
         * Close Terminal.
         */
        else if (
            terminal_open &&
            mouse_x >= terminal_x + 250 &&
            mouse_x <= terminal_x + 267 &&
            mouse_y >= terminal_y + 1 &&
            mouse_y <= terminal_y + 15
        )
        {
            terminal_open = 0;
            terminal_dragging = 0;

            redraw_all();
        }

        /*
         * Start dragging Terminal.
         */
        else if (
            terminal_open &&
            mouse_x >= terminal_x &&
            mouse_x < terminal_x + 270 &&
            mouse_y >= terminal_y &&
            mouse_y < terminal_y + 16
        )
        {
            terminal_dragging = 1;

            terminal_drag_offset_x =
                mouse_x - terminal_x;

            terminal_drag_offset_y =
                mouse_y - terminal_y;
        }
    }


    /*
     * Drag Terminal while button
     * is held.
     */
    if ((buttons & 1) &&
        terminal_dragging)
    {
        terminal_x =
            mouse_x - terminal_drag_offset_x;

        terminal_y =
            mouse_y - terminal_drag_offset_y;

        /*
         * Horizontal limits.
         */
        if (terminal_x < 0)
            terminal_x = 0;

        if (terminal_x >
            SCREEN_WIDTH - 270)
        {
            terminal_x =
                SCREEN_WIDTH - 270;
        }

        /*
         * Vertical limits.
         */
        if (terminal_y < 18)
            terminal_y = 18;

        if (terminal_y >
            SCREEN_HEIGHT - 140)
        {
            terminal_y =
                SCREEN_HEIGHT - 140;
        }

        redraw_all();
    }


    /*
     * Button released.
     */
    if (!(buttons & 1))
    {
        terminal_dragging = 0;
    }

    mouse_buttons = buttons;

    /*
     * Redraw the final cursor.
     */
    draw_cursor();
}


/* =========================
   KERNEL
   ========================= */

void kernel_main(void)
{
    mouse_init();

    terminal_open = 0;

    terminal_x = 25;
    terminal_y = 25;

    terminal_dragging = 0;

    terminal_length = 0;
    terminal_input[0] = 0;

    clear_output();

    redraw_all();

    while (1)
    {
        handle_mouse();

        if (terminal_open)
        {
            char c;

            c = keyboard_get_char();

            if (c)
            {
                /*
                 * Backspace.
                 */
                if (c == '\b')
                {
                    if (terminal_length > 0)
                    {
                        terminal_length--;

                        terminal_input[
                            terminal_length
                        ] = 0;
                    }

                    redraw_all();
                }

                /*
                 * Enter.
                 */
                else if (c == '\n')
                {
                    terminal_command();

                    terminal_length = 0;
                    terminal_input[0] = 0;

                    redraw_all();
                }

                /*
                 * Normal character.
                 */
                else if (terminal_length < 63)
                {
                    terminal_input[
                        terminal_length
                    ] = c;

                    terminal_length++;

                    terminal_input[
                        terminal_length
                    ] = 0;

                    redraw_all();
                }
            }
        }
    }
}
