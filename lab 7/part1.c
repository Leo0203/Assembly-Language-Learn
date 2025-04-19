#include <stdlib.h>
#include <math.h>
	
int pixel_buffer_start; // global variable

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();
    draw_line(0, 0, 150, 150, 0x001F);   // this line is blue
    draw_line(150, 150, 319, 0, 0x07E0); // this line is green
    draw_line(0, 239, 319, 239, 0xF800); // this line is red
    draw_line(319, 0, 0, 239, 0xF81F);   // this line is a pink color
	return 0;
}

void clear_screen()
{
	for(int x = 0; x < 320; x++){
		for(int y = 0; y < 240; y++){
			volatile short int *addr = pixel_buffer_start + (y << 10) + (x << 1);
			*addr = 0x0;
		}
	}
}

void drawLine(int startx, int starty, int endx, int endy, short int lineColor)
{
    // if the line is vertical
    if (startx == endx) {
        int y_min = (starty < endy) ? starty : endy;
        int y_max = (starty > endy) ? starty : endy;
        for (int y = y_min; y <= y_max; y++) {
            volatile short int *addr = pixel_buffer_start + (y << 10) + (startx << 1);
            *addr = lineColor;
        }
        return;
    }

    // find slope
    int dx = endx - startx;
    int dy = endy - starty;

    // if the slope is bigger than one
    int steep = (abs(dy) > abs(dx));

    if (steep) {
        // switch x and y
        int temp = startx;
        startx = starty;
        starty = temp;
        
        temp = endx;
        endx = endy;
        endy = temp;
        // recalc dx and dy
        dx = endx - startx;
        dy = endy - starty;
    }

    // always draw from left to right
    if (dx < 0) {
        int temp = startx;
        startx = endx;
        endx = temp;
        
        temp = starty;
        starty = endy;
        endy = temp;
        
        dx = -dx;
        dy = -dy;
    }

    // Bresenham
    int error = dx / 2;
    int y = starty;
    int y_step = (dy >= 0) ? 1 : -1;
    int abs_dy = abs(dy);

    for (int x = startx; x <= endx; x++) {
        if (steep) {
            //switch the axis
            volatile short int *addr = pixel_buffer_start + (x << 10) + (y << 1);
            *addr = lineColor;
        } else {
            //draw in normal axis
            volatile short int *addr = pixel_buffer_start + (y << 10) + (x << 1);
            *addr = lineColor;
        }
        error -= abs_dy;
        if (error < 0) {//error accumulate to higher than 0.5
            y += y_step;
            error += dx;    //minus one to reset the error
        }
    }
}
