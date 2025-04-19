#include <stdlib.h>
#include <math.h>

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
int boxx[8];
int boxy[8];
int dx[8];
int dy[8];
short int boxColour[8];
short int allColours[10] = {0xffff, 0x001f, 0x07e0, 0xf800, 0xf81f, 0x07ff, 0xffe0, 0xb81f, 0xa7f5, 0xb55f};

void waitForVSync();
void clear_screen();
void drawBox(int x, int y, short int boxColor);
void drawLine(int startx, int starty, int endx, int endy, short int lineColor);
int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location, direction, color of rectangles
	for(int i = 0; i < 8; i++){
		boxx[i] = rand()%320;
		boxy[i] = rand()%240;
		dx[i] = (( rand() %2) *2) - 1;
		dy[i] = (( rand() %2) *2) - 1;
		boxColour[i] = allColours[rand()%10];
	}

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    waitForVSync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
	/*
	1. front buffer(buffer1) is what the controller rendering
	2. back buffer (buffer2) is where the program draw the next frame
	3. when swap happens, buffer2 stores previous buffer1 content and 
	   buffer1 stores previous buffer2 content
	*/
    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
		//get previous location of box and lines and draw black on them
		int prevx[8];
		int prevy[8];
		for(int i = 0; i < 8; i++){
			prevx[i] = boxx[i] - dx[i];
			prevy[i] = boxy[i] - dy[i];
			drawBox(prevx[i], prevy[i], 0x0);
			
			if(i != 0){
				drawLine(prevx[i-1], prevy[i-1], prevx[i], prevy[i], 0x0);
			}
		}
		drawLine(prevx[7], prevy[7], prevx[0], prevy[0], 0x0);
		
        //update directions
		for(int i = 0; i < 8; i++){
			if(boxx[i] == 0 || boxx[i] == 316){
				dx[i] *= -1;
			}
			if(boxy[i] == 0 || boxy[i] == 236){
				dy[i] *= -1;
			}
		}
		//update new boxes and lines
		for(int i = 0; i < 8; i++){
			boxx[i] += dx[i];
			boxy[i] += dy[i];
			drawBox(boxx[i], boxy[i], boxColour[i]);
			
			if(i != 0){
				drawLine(boxx[i-1], boxy[i-1], boxx[i], boxy[i], boxColour[i]);
			}
		}
		drawLine(boxx[7], boxy[7], boxx[0], boxy[0], boxColour[0]);
        waitForVSync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer (stores previous buffer1 content)
    }
}

//clear the screen to totally black
void clear_screen()
{
	for(int x = 0; x < 320; x++){
		for(int y = 0; y < 240; y++){
			volatile short int *addr = pixel_buffer_start + (y << 10) + (x << 1);
			*addr = 0x0;
		}
	}
}

//draw a line starting from (startx, starty) and end at (endx, endy)
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
            error += dx;	//minus one to reset the error
        }
    }
}
//draw a box where (x, y) is the top left corner
void drawBox(int x, int y, short int boxColor){
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			int drawx = x + i;
			int drawy = y + j;
			volatile short int *addr = pixel_buffer_start 
			+ (drawy << 10) + (drawx << 1);
			*addr = boxColor;
		}
	}
}

//poll for next available swap of back and front buffer
void waitForVSync(){
	volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
	*(pixel_ctrl_ptr) = 1;
	while ((*(pixel_ctrl_ptr + 3) & 1) != 0) {
        // Wait for VSYNC to finish
    }
}