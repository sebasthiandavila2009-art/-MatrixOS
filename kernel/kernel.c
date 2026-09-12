// MatrixOS Kernel
// Version 2.2 - Matrix Terminal Commands

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

#define BLUE          1
#define WHITE         15
#define BLACK         0
#define LIGHT_BLUE    9
#define GREEN         10
#define GRAY          8

volatile unsigned char *framebuffer =
    (unsigned char *)0xA0000;


/* =========================================================
   Graphics
   ========================================================= */

void graphics_put_pixel(int x, int y, unsigned char color)
{
    if (x < 0 || x >= SCREEN_WIDTH ||
        y < 0 || y >= SCREEN_HEIGHT)
        return;

    framebuffer[y * SCREEN_WIDTH + x] = color;
}

void graphics_clear(unsigned char color)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

void graphics_rectangle(
    int x,
    int y,
    int width,
    int height,
    unsigned char color)
{
    for (int py = y; py < y + height; py++)
    {
        for (int px = x; px < x + width; px++)
        {
            graphics_put_pixel(px, py, color);
        }
    }
}


/* =========================================================
   5x7 Font
   ========================================================= */

static const unsigned char font[96][7] =
{
    {0,0,0,0,0,0,0},

    {4,4,4,4,4,0,4},
    {10,10,10,0,0,0,0},
    {10,31,10,31,10,0,0},
    {4,15,20,14,5,30,4},
    {24,25,2,4,8,19,3},
    {12,18,20,8,21,18,13},
    {6,4,8,0,0,0,0},

    {2,4,8,8,8,4,2},
    {8,4,2,2,2,4,8},
    {0,4,21,14,21,4,0},
    {0,4,4,31,4,4,0},
    {0,0,0,0,6,4,8},
    {0,0,0,31,0,0,0},
    {0,0,0,0,0,6,6},
    {0,1,2,4,8,16,0},

    {14,17,19,21,25,17,14},
    {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31},
    {31,2,4,2,1,17,14},
    {2,6,10,18,31,2,2},
    {31,16,30,1,1,17,14},
    {6,8,16,30,17,17,14},
    {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14},
    {14,17,17,15,1,2,12},
    {0,6,6,0,6,6,0},
    {0,6,6,0,6,4,8},
    {2,4,8,16,8,4,2},
    {0,0,31,0,31,0,0},
    {8,4,2,1,2,4,8},
    {14,17,1,2,4,0,4},

    {14,17,1,13,21,21,14},
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
    {31,1,2,4,8,16,31},

    {14,8,8,8,8,8,14},
    {0,16,8,4,2,1,0},
    {14,2,2,2,2,2,14},
    {4,10,17,0,0,0,0},
    {0,0,0,0,0,0,31},
    {8,4,2,0,0,0,0},

    {0,0,14,1,15,17,15},
    {16,16,22,25,17,17,30},
    {0,0,14,17,16,17,14},
    {1,1,13,19,17,17,15},
    {0,0,14,17,31,16,14},
    {6,9,8,28,8,8,8},
    {0,0,15,17,17,15,1},
    {16,16,22,25,17,17,17},
    {4,0,12,4,4,4,14},
    {2,0,6,2,2,18,12},
    {16,16,18,20,24,20,18},
    {12,4,4,4,4,4,14},
    {0,0,26,21,21,17,17},
    {0,0,30,17,17,17,17},
    {0,0,14,17,17,17,14},
    {0,0,30,17,17,30,16},
    {0,0,13,19,17,15,1},
    {0,0,22,25,16,16,16},
    {0,0,15,16,14,1,30},
    {8,8,28,8,8,9,6},
    {0,0,17,17,17,19,13},
    {0,0,17,17,17,10,4},
    {0,0,17,17,21,21,10},
    {0,0,17,10,4,10,17},
    {0,0,17,17,15,1,14},
    {0,0,31,2,4,8,31},

    {2,4,8,8,8,4,2},
    {4,4,4,4,4,4,4},
    {8,4,2,2,2,4,8},
    {8,21,2,0,0,0,0},
    {31,31,31,31,31,31,31}
};


/* =========================================================
   Text
   ========================================================= */

void draw_character_at(
    int x,
    int y,
    char c,
    unsigned char color)
{
    if (c < 32 || c > 127)
        return;

    const unsigned char *glyph = font[c - 32];

    for (int row = 0; row < 7; row++)
    {
        for (int col = 0; col < 5; col++)
        {
            if (glyph[row] & (1 << (4 - col)))
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

void draw_text(
    int x,
    int y,
    const char *text,
    unsigned char color)
{
    while (*text)
    {
        draw_character_at(x, y, *text, color);
        x += 6;
        text++;
    }
}


/* =========================================================
   Desktop
   ========================================================= */

void draw_terminal_icon(int x, int y)
{
    graphics_rectangle(x, y, 42, 32, BLACK);
    graphics_rectangle(x + 2, y + 2, 38, 28, WHITE);

    draw_character_at(x + 7, y + 9, '>', BLACK);
    draw_character_at(x + 14, y + 9, '_', BLACK);
}

void draw_files_icon(int x, int y)
{
    graphics_rectangle(x, y + 5, 42, 27, WHITE);
    graphics_rectangle(x + 5, y, 19, 8, WHITE);
}

void draw_settings_icon(int x, int y)
{
    graphics_rectangle(x + 5, y + 5, 32, 22, GRAY);
    graphics_rectangle(x + 13, y + 9, 16, 14, BLACK);
}

void draw_about_icon(int x, int y)
{
    graphics_rectangle(x, y, 42, 32, LIGHT_BLUE);
    draw_character_at(x + 18, y + 5, 'i', WHITE);
}

void draw_desktop(void)
{
    graphics_clear(BLUE);

    /* Top bar */
    graphics_rectangle(
        0, 0,
        SCREEN_WIDTH, 18,
        BLACK
    );

    draw_text(
        8, 5,
        "MATRIXBOOK",
        WHITE
    );

    draw_text(
        250, 5,
        "MATRIXOS",
        WHITE
    );

    /* App icons */
    draw_terminal_icon(20, 35);
    draw_files_icon(100, 35);
    draw_settings_icon(180, 35);
    draw_about_icon(260, 35);

    draw_text(20, 72, "TERMINAL", WHITE);
    draw_text(104, 72, "FILES", WHITE);
    draw_text(181, 72, "SETTINGS", WHITE);
    draw_text(266, 72, "ABOUT", WHITE);

    /* Dock */
    graphics_rectangle(
        45, 170,
        230, 25,
        BLACK
    );

    draw_text(
        58, 179,
        "TERMINAL",
        WHITE
    );
}


/* =========================================================
   Terminal
   ========================================================= */

static int terminal_open = 0;

static char terminal_input[64];
static int terminal_length = 0;

void terminal_draw(void)
{
    /* Window */
    graphics_rectangle(
        25, 25,
        270, 140,
        WHITE
    );

    /* Title bar */
    graphics_rectangle(
        25, 25,
        270, 16,
        BLACK
    );

    draw_text(
        33, 30,
        "MATRIX TERMINAL",
        WHITE
    );

    /* Close button */
    graphics_rectangle(
        279, 29,
        10, 8,
        GRAY
    );

    /* Terminal background */
    graphics_rectangle(
        30, 45,
        260, 115,
        BLACK
    );

    draw_text(
        37, 52,
        "MatrixOS Terminal",
        GREEN
    );

    draw_text(
        37, 64,
        "Type 'help' for commands.",
        WHITE
    );

    draw_text(
        37, 82,
        "matrix@matrixbook:~$",
        GREEN
    );

    draw_text(
        37 + (20 * 6),
        82,
        terminal_input,
        WHITE
    );

    /* Cursor */
    graphics_rectangle(
        37 + (20 * 6) +
        (terminal_length * 6),
        81,
        5,
        8,
        WHITE
    );
}


/* =========================================================
   Terminal Commands
   ========================================================= */

void terminal_print_command(const char *command)
{
    if (command[0] == 'h' &&
        command[1] == 'e' &&
        command[2] == 'l' &&
        command[3] == 'p' &&
        command[4] == 0)
    {
        graphics_rectangle(
            30, 95,
            260, 65,
            BLACK
        );

        draw_text(
            37, 98,
            "MATRIXOS COMMANDS",
            GREEN
        );

        draw_text(
            37, 110,
            "help",
            WHITE
        );

        draw_text(
            80, 110,
            "- Show commands",
            WHITE
        );

        draw_text(
            37, 122,
            "about",
            WHITE
        );

        draw_text(
            80, 122,
            "- About MatrixOS",
            WHITE
        );

        draw_text(
            37, 134,
            "clear",
            WHITE
        );

        draw_text(
            80, 134,
            "- Clear terminal",
            WHITE
        );

        draw_text(
            37, 146,
            "exit",
            WHITE
        );

        draw_text(
            80, 146,
            "- Close terminal",
            WHITE
        );

        return;
    }

    if (command[0] == 'a' &&
        command[1] == 'b' &&
        command[2] == 'o' &&
        command[3] == 'u' &&
        command[4] == 't' &&
        command[5] == 0)
    {
        graphics_rectangle(
            30, 95,
            260, 65,
            BLACK
        );

        draw_text(
            37, 100,
            "MATRIXOS",
            GREEN
        );

        draw_text(
            37, 112,
            "MatrixBook Desktop",
            WHITE
        );

        draw_text(
            37, 124,
            "Version 2.2",
            WHITE
        );

        draw_text(
            37, 136,
            "Built by MatrixOS Co.",
            WHITE
        );

        return;
    }

    if (command[0] == 'c' &&
        command[1] == 'l' &&
        command[2] == 'e' &&
        command[3] == 'a' &&
        command[4] == 'r' &&
        command[5] == 0)
    {
        graphics_rectangle(
            30, 95,
            260, 65,
            BLACK
        );

        return;
    }

    if (command[0] == 'e' &&
        command[1] == 'x' &&
        command[2] == 'i' &&
        command[3] == 't' &&
        command[4] == 0)
    {
        terminal_open = 0;
        draw_desktop();
        return;
    }

    graphics_rectangle(
        30, 95,
        260, 65,
        BLACK
    );

    draw_text(
        37, 105,
        "Command not found.",
        WHITE
    );

    draw_text(
        37, 117,
        "Type 'help'.",
        GREEN
    );
}


/* =========================================================
   Keyboard
   ========================================================= */

extern char keyboard_get_char(void);


/* =========================================================
   Mouse
   ========================================================= */

extern void mouse_init(void);

extern int mouse_get_packet(
    int *dx,
    int *dy,
    unsigned char *buttons
);

static int mouse_x = 160;
static int mouse_y = 100;

static unsigned char mouse_buttons = 0;
static unsigned char previous_buttons = 0;


/* =========================================================
   Mouse Cursor
   ========================================================= */

void draw_cursor(void)
{
    for (int i = 0; i < 8; i++)
    {
        graphics_put_pixel(
            mouse_x,
            mouse_y + i,
            WHITE
        );

        if (i < 5)
        {
            graphics_put_pixel(
                mouse_x + i,
                mouse_y + i,
                WHITE
            );
        }
    }
}


/* =========================================================
   Main Kernel
   ========================================================= */

void kernel_main(void)
{
    graphics_clear(BLUE);

    mouse_init();

    draw_desktop();

    draw_cursor();

    while (1)
    {
        int dx;
        int dy;
        unsigned char buttons;

        if (mouse_get_packet(
                &dx,
                &dy,
                &buttons))
        {
            mouse_x += dx;
            mouse_y -= dy;

            if (mouse_x < 0)
                mouse_x = 0;

            if (mouse_x >= SCREEN_WIDTH)
                mouse_x = SCREEN_WIDTH - 1;

            if (mouse_y < 18)
                mouse_y = 18;

            if (mouse_y >= SCREEN_HEIGHT)
                mouse_y = SCREEN_HEIGHT - 1;

            mouse_buttons = buttons;

            /* Left-click */
            if ((mouse_buttons & 1) &&
                !(previous_buttons & 1))
            {
                if (!terminal_open &&
                    mouse_x >= 15 &&
                    mouse_x <= 70 &&
                    mouse_y >= 30 &&
                    mouse_y <= 75)
                {
                    terminal_open = 1;
                    terminal_length = 0;
                    terminal_input[0] = 0;

                    terminal_draw();
                }

                else if (terminal_open &&
                         mouse_x >= 275 &&
                         mouse_x <= 292 &&
                         mouse_y >= 25 &&
                         mouse_y <= 45)
                {
                    terminal_open = 0;
                    draw_desktop();
                }
            }

            previous_buttons = mouse_buttons;

            if (terminal_open)
                terminal_draw();
            else
                draw_desktop();

            draw_cursor();
        }

        if (terminal_open)
        {
            char c = keyboard_get_char();

            if (c)
            {
                if (c == '\b')
                {
                    if (terminal_length > 0)
                    {
                        terminal_length--;

                        terminal_input[
                            terminal_length
                        ] = 0;
                    }
                }

                else if (c == '\n')
                {
                    terminal_print_command(
                        terminal_input
                    );

                    terminal_length = 0;
                    terminal_input[0] = 0;
                }

                else if (terminal_length < 63)
                {
                    terminal_input[
                        terminal_length
                    ] = c;

                    terminal_length++;

                    terminal_input[
                        terminal_length
                    ] = 0;
                }

                terminal_draw();
                draw_cursor();
            }
        }
    }
}
