// MatrixOS Graphics Driver
// Version 0.2 - Shared framebuffer with kernel

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

/*
 * The framebuffer is owned by the kernel.
 * Do not define it again here.
 */
extern volatile unsigned char *framebuffer;

/*
 * Draw one pixel.
 */
void graphics_put_pixel(int x, int y, unsigned char color)
{
    if (x < 0 || x >= SCREEN_WIDTH ||
        y < 0 || y >= SCREEN_HEIGHT)
    {
        return;
    }

    framebuffer[y * SCREEN_WIDTH + x] = color;
}

/*
 * Fill the entire screen with one color.
 */
void graphics_clear(unsigned char color)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            graphics_put_pixel(x, y, color);
        }
    }
}

/*
 * Draw a rectangle.
 */
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
