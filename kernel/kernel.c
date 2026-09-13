// MatrixOS Kernel
// Version 2.5 - Mouse Enabled Desktop

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define BLUE 1
#define WHITE 15
#define BLACK 0
#define LIGHT_BLUE 9
#define GREEN 10
#define GRAY 8

extern void graphics_put_pixel(int x, int y, unsigned char color);
extern void graphics_clear(unsigned char color);
extern void graphics_rectangle(int x, int y, int width, int height, unsigned char color);
extern char keyboard_get_char(void);
extern void mouse_init(void);
extern int mouse_get_packet(int *dx, int *dy, unsigned char *buttons);

/* Compact 5x7 uppercase font. */
static const unsigned char font[26][7] = {
    {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
    {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
    {14,17,16,23,17,17,14},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
    {7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
    {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
    {14,17,16,14,1,17,14},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
    {17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
};

static const unsigned char digits[10][7] = {
    {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
    {31,2,4,2,1,17,14},{2,6,10,18,31,2,2},{31,16,30,1,1,17,14},
    {6,8,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
    {14,17,17,15,1,2,12}
};

static void draw_char(int x, int y, char c, unsigned char color)
{
    const unsigned char *g = 0;
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') g = font[c - 'A'];
    else if (c >= '0' && c <= '9') g = digits[c - '0'];

    if (g)
    {
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 5; col++)
                if (g[row] & (1 << (4 - col)))
                    graphics_put_pixel(x + col, y + row, color);
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

static void draw_text(int x, int y, const char *text, unsigned char color)
{
    while (*text)
    {
        draw_char(x, y, *text, color);
        x += 6;
        text++;
    }
}

static int string_equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

/* Desktop state. */
static int terminal_open = 0;
static char terminal_input[64];
static int terminal_length = 0;

static int mouse_x = 160;
static int mouse_y = 100;
static unsigned char mouse_buttons = 0;

static void draw_terminal_icon(int x, int y)
{
    graphics_rectangle(x, y, 42, 32, BLACK);
    graphics_rectangle(x + 2, y + 2, 38, 28, WHITE);
    draw_char(x + 7, y + 9, '>', BLACK);
    draw_char(x + 14, y + 9, '_', BLACK);
}

static void draw_files_icon(int x, int y)
{
    graphics_rectangle(x, y + 5, 42, 27, WHITE);
    graphics_rectangle(x + 5, y, 19, 8, WHITE);
}

static void draw_settings_icon(int x, int y)
{
    graphics_rectangle(x + 5, y + 5, 32, 22, GRAY);
    graphics_rectangle(x + 13, y + 9, 16, 14, BLACK);
}

static void draw_about_icon(int x, int y)
{
    graphics_rectangle(x, y, 42, 32, LIGHT_BLUE);
    draw_char(x + 18, y + 5, 'I', WHITE);
}

static void draw_terminal_window(void)
{
    if (!terminal_open) return;

    graphics_rectangle(28, 28, 270, 140, BLACK);
    graphics_rectangle(25, 25, 270, 140, WHITE);
    graphics_rectangle(25, 25, 270, 16, BLACK);
    draw_text(33, 30, "MATRIX TERMINAL", WHITE);
    graphics_rectangle(25, 45, 270, 120, BLACK);

    draw_text(37, 52, "MATRIXOS TERMINAL", GREEN);
    draw_text(37, 64, "TYPE HELP FOR COMMANDS", WHITE);
    draw_text(37, 82, "MATRIXBOOK $", GREEN);
    draw_text(115, 82, terminal_input, WHITE);
    graphics_rectangle(115 + terminal_length * 6, 81, 5, 8, WHITE);
}

static void draw_desktop(void)
{
    graphics_clear(BLUE);
    graphics_rectangle(0, 0, SCREEN_WIDTH, 18, BLACK);
    draw_text(8, 5, "MATRIXBOOK", WHITE);
    draw_text(250, 5, "MATRIXOS", WHITE);

    draw_terminal_icon(20, 35);
    draw_files_icon(100, 35);
    draw_settings_icon(180, 35);
    draw_about_icon(260, 35);

    draw_text(20, 72, "TERMINAL", WHITE);
    draw_text(104, 72, "FILES", WHITE);
    draw_text(181, 72, "SETTINGS", WHITE);
    draw_text(266, 72, "ABOUT", WHITE);

    graphics_rectangle(45, 170, 230, 25, BLACK);
    draw_text(58, 179, "TERMINAL", WHITE);

    draw_terminal_window();
}

static void draw_cursor(void)
{
    for (int i = 0; i < 9; i++)
    {
        graphics_put_pixel(mouse_x, mouse_y + i, WHITE);
        if (i < 6)
            graphics_put_pixel(mouse_x + i, mouse_y + i, WHITE);
    }
}

static void redraw(void)
{
    draw_desktop();
    draw_cursor();
}

static void handle_mouse(void)
{
    int dx, dy;
    unsigned char buttons;

    if (!mouse_get_packet(&dx, &dy, &buttons))
        return;

    mouse_x += dx;
    mouse_y -= dy;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x > SCREEN_WIDTH - 2) mouse_x = SCREEN_WIDTH - 2;
    if (mouse_y < 18) mouse_y = 18;
    if (mouse_y > SCREEN_HEIGHT - 9) mouse_y = SCREEN_HEIGHT - 9;

    if ((buttons & 1) && !(mouse_buttons & 1))
    {
        if (mouse_x >= 20 && mouse_x < 62 && mouse_y >= 35 && mouse_y < 67)
        {
            terminal_open = 1;
            terminal_length = 0;
            terminal_input[0] = 0;
        }
        else if (terminal_open && mouse_x >= 280 && mouse_x < 295 && mouse_y >= 25 && mouse_y < 45)
        {
            terminal_open = 0;
        }
    }

    mouse_buttons = buttons;
    redraw();
}

static void terminal_command(void)
{
    if (string_equal(terminal_input, "help"))
    {
        draw_text(37, 100, "HELP  ABOUT  CLEAR  EXIT", WHITE);
    }
    else if (string_equal(terminal_input, "about"))
    {
        draw_text(37, 100, "MATRIXOS VERSION 2.5", GREEN);
        draw_text(37, 112, "MATRIXBOOK DESKTOP", WHITE);
    }
    else if (string_equal(terminal_input, "clear"))
    {
        graphics_rectangle(30, 95, 260, 60, BLACK);
    }
    else if (string_equal(terminal_input, "exit"))
    {
        terminal_open = 0;
    }
}

void kernel_main(void)
{
    mouse_init();
    redraw();

    while (1)
    {
        handle_mouse();

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
                        terminal_input[terminal_length] = 0;
                    }
                }
                else if (c == '\n')
                {
                    terminal_command();
                    terminal_length = 0;
                    terminal_input[0] = 0;
                }
                else if (terminal_length < 63)
                {
                    terminal_input[terminal_length++] = c;
                    terminal_input[terminal_length] = 0;
                }

                redraw();
            }
        }
    }
}
