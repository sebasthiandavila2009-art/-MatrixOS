// MatrixOS Kernel
// Version 2.8 - Proper Mouse Cursor

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define BLUE 1
#define WHITE 15
#define BLACK 0
#define LIGHT_BLUE 9
#define GREEN 10
#define RED 4
#define GRAY 8

#define CURSOR_WIDTH 9
#define CURSOR_HEIGHT 9

/*
 * VGA Mode 13h framebuffer.
 *
 * We use this directly for the cursor backing buffer.
 */
volatile unsigned char *framebuffer =
    (unsigned char *)0xA0000;

extern void graphics_put_pixel(
    int x,
    int y,
    unsigned char color
);

extern void graphics_clear(
    unsigned char color
);

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

extern void matrixfs_init(void);

extern int matrixfs_list(
    const char *parent,
    int *results,
    int max_results
);

extern int matrixfs_is_directory(int index);

extern const char *matrixfs_name(int index);

extern const char *matrixfs_read(int index);


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
        draw_char(
            x,
            y,
            *text,
            color
        );

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

/* =========================
   FILES STATE
   ========================= */

#define FILES_WIDTH 230
#define FILES_HEIGHT 135

static int files_open = 0;
static int files_x = 45;
static int files_y = 30;
static int files_dragging = 0;
static int files_drag_offset_x = 0;
static int files_drag_offset_y = 0;

static const char *files_current_dir = "/";
static int files_entries[16];
static int files_entry_count = 0;
static int files_viewing_file = 0;
static int files_viewed_entry = -1;
static int files_click_armed = 0;

static int terminal_open = 0;

static int terminal_x = 25;
static int terminal_y = 25;

static int terminal_dragging = 0;

static int terminal_drag_offset_x = 0;
static int terminal_drag_offset_y = 0;

static char terminal_input[64];
static int terminal_length = 0;


/* Terminal output. */
static char output_line_1[43];
static char output_line_2[43];
static char output_line_3[43];


/* =========================
   MOUSE STATE
   ========================= */

static int mouse_x = 160;
static int mouse_y = 100;

static unsigned char mouse_buttons = 0;
static unsigned char previous_mouse_buttons = 0;


/*
 * Cursor backing store.
 *
 * Before drawing the cursor, we save the
 * pixels underneath it.
 *
 * Before moving it, we restore those pixels.
 */
static unsigned char cursor_background[
    CURSOR_WIDTH * CURSOR_HEIGHT
];

static int cursor_visible = 0;


/* =========================
   CURSOR BACKGROUND
   ========================= */

static void save_cursor_background(void)
{
    for (int y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (int x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = mouse_x + x;
            int py = mouse_y + y;

            if (px >= 0 &&
                px < SCREEN_WIDTH &&
                py >= 0 &&
                py < SCREEN_HEIGHT)
            {
                cursor_background[
                    y * CURSOR_WIDTH + x
                ] =
                    framebuffer[
                        py * SCREEN_WIDTH + px
                    ];
            }
            else
            {
                cursor_background[
                    y * CURSOR_WIDTH + x
                ] = BLACK;
            }
        }
    }
}

static void restore_cursor_background(void)
{
    if (!cursor_visible)
        return;

    for (int y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (int x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = mouse_x + x;
            int py = mouse_y + y;

            if (px >= 0 &&
                px < SCREEN_WIDTH &&
                py >= 0 &&
                py < SCREEN_HEIGHT)
            {
                framebuffer[
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


/* =========================
   CURSOR
   ========================= */

static void draw_cursor(void)
{
    save_cursor_background();

    /*
     * Arrow cursor.
     */
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

    cursor_visible = 1;
}


/* =========================
   OUTPUT
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


/* =========================
   TERMINAL COMMANDS
   ========================= */

static void terminal_command(void)
{
    clear_output();

    if (string_equal(
            terminal_input,
            "help"))
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
    else if (string_equal(
                 terminal_input,
                 "about"))
    {
        copy_text(
            output_line_1,
            "MATRIXOS VERSION 2.8"
        );

        copy_text(
            output_line_2,
            "MATRIXBOOK DESKTOP"
        );
    }
    else if (string_equal(
                 terminal_input,
                 "clear"))
    {
        clear_output();
    }
    else if (string_equal(
                 terminal_input,
                 "exit"))
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
   DESKTOP ICONS
   ========================= */

static void draw_files_close_button(void) {
    int center_x = files_x + FILES_WIDTH - 12;
    int center_y = files_y + 8;

    for (int y = -4; y <= 4; y++) {
        for (int x = -4; x <= 4; x++) {
            if ((x * x) + (y * y) <= 16) {
                graphics_put_pixel(center_x + x, center_y + y, RED);
            }
        }
    }
}

static void draw_folder_icon(int x, int y) {
    graphics_rectangle(x, y + 4, 18, 14, WHITE);
    graphics_rectangle(x + 3, y, 9, 5, WHITE);
}

static void files_open_entry(int entry)
{
    if (matrixfs_is_directory(entry))
    {
        files_current_dir = matrixfs_name(entry);
        files_viewing_file = 0;
        files_viewed_entry = -1;
    }
    else
    {
        files_viewing_file = 1;
        files_viewed_entry = entry;
    }
}

static void files_go_back(void)
{
    files_viewing_file = 0;
    files_viewed_entry = -1;

    if (!string_equal(files_current_dir, "/"))
        files_current_dir = "/";
}

static void draw_files_window(void)
{
    int x = files_x;
    int y = files_y;

    if (!files_open)
        return;

    graphics_rectangle(x + 3, y + 3, FILES_WIDTH, FILES_HEIGHT, BLACK);
    graphics_rectangle(x, y, FILES_WIDTH, FILES_HEIGHT, WHITE);
    graphics_rectangle(x, y, FILES_WIDTH, 16, BLACK);

    draw_text(
        x + 8,
        y + 5,
        "MATRIX FILES",
        WHITE
    );

    draw_files_close_button();

    graphics_rectangle(
        x,
        y + 20,
        FILES_WIDTH,
        FILES_HEIGHT - 20,
        BLACK
    );

    if (files_viewing_file && files_viewed_entry >= 0)
    {
        draw_text(
            x + 12,
            y + 28,
            matrixfs_name(files_viewed_entry),
            GREEN
        );

        draw_text(
            x + 12,
            y + 48,
            matrixfs_read(files_viewed_entry),
            WHITE
        );

        draw_text(
            x + 12,
            y + 105,
            "BACK",
            GRAY
        );

        return;
    }

    if (!string_equal(files_current_dir, "/"))
    {
        draw_text(
            x + 12,
            y + 27,
            "< BACK",
            GRAY
        );
    }

    files_entry_count = matrixfs_list(
        files_current_dir,
        files_entries,
        16
    );

    for (int i = 0; i < files_entry_count; i++)
    {
        int entry = files_entries[i];
        int row_y = y + 43 + (i * 25);

        if (row_y + 12 >= y + FILES_HEIGHT)
            break;

        if (matrixfs_is_directory(entry))
        {
            draw_folder_icon(
                x + 12,
                row_y
            );
        }
        else
        {
            graphics_rectangle(
                x + 13,
                row_y,
                16,
                19,
                WHITE
            );

            graphics_rectangle(
                x + 17,
                row_y + 4,
                9,
                1,
                BLACK
            );

            graphics_rectangle(
                x + 17,
                row_y + 8,
                9,
                1,
                BLACK
            );

            graphics_rectangle(
                x + 17,
                row_y + 12,
                7,
                1,
                BLACK
            );
        }

        draw_text(
            x + 38,
            row_y + 4,
            matrixfs_name(entry),
            WHITE
        );
    }

    if (files_entry_count == 0)
    {
        draw_text(
            x + 12,
            y + 50,
            "EMPTY",
            GRAY
        );
    }
}

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
    int center_x =
        terminal_x + 258;

    int center_y =
        terminal_y + 8;

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
    draw_files_window();
}


/* =========================
   FULL SCREEN REDRAW
   ========================= */

static void redraw_all(void)
{
    /*
     * Remove the old cursor first.
     */
    restore_cursor_background();

    /*
     * Redraw everything underneath it.
     */
    draw_desktop();

    /*
     * Put cursor back on top.
     */
    draw_cursor();
}


/* =========================
   MOUSE MOVEMENT
   ========================= */

static void move_cursor(
    int dx,
    int dy
)
{
    /*
     * Remove old cursor.
     */
    restore_cursor_background();

    mouse_x += dx;
    mouse_y -= dy;

    /*
     * Horizontal limits.
     */
    if (mouse_x < 0)
        mouse_x = 0;

    if (mouse_x >
        SCREEN_WIDTH - CURSOR_WIDTH)
    {
        mouse_x =
            SCREEN_WIDTH - CURSOR_WIDTH;
    }

    /*
     * Vertical limits.
     */
    if (mouse_y < 18)
        mouse_y = 18;

    if (mouse_y >
        SCREEN_HEIGHT - CURSOR_HEIGHT)
    {
        mouse_y =
            SCREEN_HEIGHT - CURSOR_HEIGHT;
    }

    /*
     * Draw cursor at new position.
     */
    draw_cursor();
}


/* =========================
   MOUSE HANDLER
   ========================= */

static void handle_mouse(void)
{
    int dx;
    int dy;
    unsigned char buttons;

    if (!mouse_get_packet(&dx, &dy, &buttons))
        return;

    /*
     * A click is ONLY a 0 -> 1 transition.
     *
     * This is deliberately calculated once for
     * the entire mouse packet. Hovering and motion
     * can never become a click.
     */
    int left_pressed =
        ((buttons & 1) != 0) &&
        ((mouse_buttons & 1) == 0);

    int left_released =
        ((buttons & 1) == 0) &&
        ((mouse_buttons & 1) != 0);

    /*
     * Move cursor first.
     */
    if (dx != 0 || dy != 0)
    {
        move_cursor(dx, dy);
    }

    /*
     * Mouse release.
     *
     * A release ends dragging and arms Files
     * for the NEXT real press.
     */
    if (left_released)
    {
        terminal_dragging = 0;
        files_dragging = 0;

        if (files_open)
            files_click_armed = 1;
    }

    /*
     * =====================================================
     * FILES WINDOW
     * =====================================================
     */

    if (files_open)
    {
        /*
         * Drag an already-active Files window.
         */
        if (buttons & 1 && files_dragging)
        {
            restore_cursor_background();

            files_x = mouse_x - files_drag_offset_x;
            files_y = mouse_y - files_drag_offset_y;

            if (files_x < 0)
                files_x = 0;

            if (files_x > SCREEN_WIDTH - FILES_WIDTH)
                files_x = SCREEN_WIDTH - FILES_WIDTH;

            if (files_y < 18)
                files_y = 18;

            if (files_y > SCREEN_HEIGHT - FILES_HEIGHT)
                files_y = SCREEN_HEIGHT - FILES_HEIGHT;

            draw_desktop();
            draw_cursor();
        }

        /*
         * ONLY a real press can interact with Files.
         */
        if (left_pressed && files_click_armed)
        {
            /*
             * Close button.
             */
            if (mouse_x >= files_x + FILES_WIDTH - 22 &&
                mouse_x <= files_x + FILES_WIDTH - 3 &&
                mouse_y >= files_y + 1 &&
                mouse_y <= files_y + 15)
            {
                files_open = 0;
                files_dragging = 0;
                files_click_armed = 0;

                redraw_all();
            }

            /*
             * Title bar.
             */
            else if (mouse_x >= files_x &&
                     mouse_x < files_x + FILES_WIDTH &&
                     mouse_y >= files_y &&
                     mouse_y < files_y + 16)
            {
                files_dragging = 1;

                files_drag_offset_x =
                    mouse_x - files_x;

                files_drag_offset_y =
                    mouse_y - files_y;

                files_click_armed = 0;
            }

            /*
             * Files contents.
             */
            else
            {
                /*
                 * Rebuild the visible entry list.
                 */
                files_entry_count =
                    matrixfs_list(
                        files_current_dir,
                        files_entries,
                        16
                    );

                /*
                 * BACK while inside a directory.
                 */
                if (!string_equal(files_current_dir, "/") &&
                    mouse_x >= files_x &&
                    mouse_x < files_x + 100 &&
                    mouse_y >= files_y + 20 &&
                    mouse_y < files_y + 43)
                {
                    files_click_armed = 0;
                    files_current_dir = "/";
                    files_viewing_file = 0;
                    files_viewed_entry = -1;

                    redraw_all();
                }

                /*
                 * File view BACK.
                 */
                else if (files_viewing_file &&
                         mouse_x >= files_x &&
                         mouse_x < files_x + 100 &&
                         mouse_y >= files_y + 95 &&
                         mouse_y < files_y + FILES_HEIGHT)
                {
                    files_click_armed = 0;
                    files_viewing_file = 0;
                    files_viewed_entry = -1;

                    redraw_all();
                }

                /*
                 * Entry click.
                 */
                else if (!files_viewing_file)
                {
                    for (int i = 0;
                         i < files_entry_count;
                         i++)
                    {
                        int entry = files_entries[i];

                        int row_y =
                            files_y + 31 + (i * 25);

                        if (mouse_x < files_x + 8 ||
                            mouse_x >= files_x + FILES_WIDTH - 8)
                            continue;

                        if (mouse_y < row_y - 3 ||
                            mouse_y >= row_y + 21)
                            continue;

                        /*
                         * Consume this press immediately.
                         */
                        files_click_armed = 0;

                        if (matrixfs_is_directory(entry))
                        {
                            files_current_dir =
                                matrixfs_name(entry);

                            files_viewing_file = 0;
                            files_viewed_entry = -1;
                        }
                        else
                        {
                            files_viewing_file = 1;
                            files_viewed_entry = entry;
                        }

                        redraw_all();
                        break;
                    }
                }
            }
        }
    }

    /*
     * =====================================================
     * OPEN FILES FROM DESKTOP
     * =====================================================
     *
     * This is outside the Files window so that the
     * launch click cannot also activate a Files entry.
     */
    if (!files_open &&
        !terminal_open &&
        left_pressed &&
        mouse_x >= 100 &&
        mouse_x < 142 &&
        mouse_y >= 35 &&
        mouse_y < 67)
    {
        files_open = 1;
        files_dragging = 0;

        /*
         * Consume the launch click.
         */
        files_click_armed = 0;

        redraw_all();
    }

    /*
     * =====================================================
     * TERMINAL
     * =====================================================
     */

    if (terminal_open)
    {
        /*
         * Drag Terminal.
         */
        if ((buttons & 1) && terminal_dragging)
        {
            restore_cursor_background();

            terminal_x =
                mouse_x - terminal_drag_offset_x;

            terminal_y =
                mouse_y - terminal_drag_offset_y;

            if (terminal_x < 0)
                terminal_x = 0;

            if (terminal_x > SCREEN_WIDTH - 270)
                terminal_x = SCREEN_WIDTH - 270;

            if (terminal_y < 18)
                terminal_y = 18;

            if (terminal_y > SCREEN_HEIGHT - 140)
                terminal_y = SCREEN_HEIGHT - 140;

            draw_desktop();
            draw_cursor();
        }

        /*
         * Terminal controls only react to a real press.
         */
        if (left_pressed)
        {
            /*
             * Close Terminal.
             */
            if (mouse_x >= terminal_x + 250 &&
                mouse_x <= terminal_x + 267 &&
                mouse_y >= terminal_y + 1 &&
                mouse_y <= terminal_y + 15)
            {
                terminal_open = 0;
                terminal_dragging = 0;

                redraw_all();
            }

            /*
             * Terminal title bar.
             */
            else if (mouse_x >= terminal_x &&
                     mouse_x < terminal_x + 270 &&
                     mouse_y >= terminal_y &&
                     mouse_y < terminal_y + 16)
            {
                terminal_dragging = 1;

                terminal_drag_offset_x =
                    mouse_x - terminal_x;

                terminal_drag_offset_y =
                    mouse_y - terminal_y;
            }
        }
    }

    /*
     * Open Terminal from desktop.
     *
     * Files gets checked first above, so these
     * desktop icons do not share the same click.
     */
    if (!files_open &&
        !terminal_open &&
        left_pressed &&
        mouse_x >= 20 &&
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
     * The current packet becomes the previous state
     * ONLY after every click decision has been made.
     */
    previous_mouse_buttons = buttons;
    mouse_buttons = buttons;
}

/* =========================
   KERNEL
   ========================= */

void kernel_main(void)
{
    matrixfs_init();
    mouse_init();

    terminal_open = 0;

    files_open = 0;
    files_x = 45;
    files_y = 30;
    files_dragging = 0;

    files_current_dir = "/";
    files_viewing_file = 0;
    files_viewed_entry = -1;
    files_click_armed = 0;
    previous_mouse_buttons = 0;
    mouse_buttons = 0;

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
                 * Remove cursor before changing
                 * terminal contents.
                 */
                restore_cursor_background();

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
                }

                /*
                 * Enter.
                 */
                else if (c == '\n')
                {
                    terminal_command();

                    terminal_length = 0;
                    terminal_input[0] = 0;
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
                }

                redraw_all();
            }
        }
    }
}
