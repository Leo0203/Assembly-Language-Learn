#include <stdlib.h>
#include <math.h>
	
int pixel_buffer_start; // global variable

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = (*pixel_ctrl_ptr);
	*(pixel_ctrl_ptr + 1) = pixel_buffer_start;
	
	//initial setup
    clear_screen();
	int y = 119;
    draw_line(0, y, 319, y, 0xffff);
	waitForVSync();
	int isMoveUp = 1;
	
	while(1){
		if(isMoveUp){
			y--;
			clear_screen();
			draw_line(0, y, 319, y, 0xffff);
			waitForVSync();
		}else{
			y++;
			clear_screen();
			draw_line(0, y, 319, y, 0xffff);
			waitForVSync();
		}
		//check if boundaries are met
		if(y == 0 || y == 239){
			isMoveUp ^= 1;
		}
	}
	
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

void draw_line(int startx, int starty, int endx, int endy, short int lineColor)
{
	if(endx == startx){//vertical line
		if(starty > endy){
			for(int curry = starty; curry >= endy; curry--){
				volatile short int *addr = pixel_buffer_start 
							+ (curry << 10) + (startx << 1);
				*addr = lineColor;
			}
			return;
		}else{
			for(int curry = endy; curry >= starty; curry--){
				volatile short int *addr = pixel_buffer_start 
							+ (curry << 10) + (startx << 1);
				*addr = lineColor;
			}
			return;
		}
	}
	double slope = (double)(endy - starty) / (endx - startx);
	if(slope > 1 || slope < -1){//slope is steep
		slope = 1 / slope;
		//switch x and y axis
		int temp = startx;
		startx = starty;
		starty = temp;
		
		temp = endx;
		endx = endy;
		endy = temp;
	}
	//start drawing
	//always draw from left to right
	if(endx < startx){
		int temp = startx;
		startx = endx;
		endx = temp;

		temp = starty;
		starty = endy;
		endy = temp;
	}
	int numBlock = endx - startx + 1;
	for(int i = 0; i < numBlock; i++){
		int drawx = startx + i;
		double actualy = starty + slope*(drawx - startx);
		int drawy = round(actualy);
		volatile short int *addr = pixel_buffer_start 
			+ (drawy << 10) + (drawx << 1);
		*addr = lineColor;
	}
	return;
	
}

void waitForVSync(){
	volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
	*(pixel_ctrl_ptr) = 1;
	while ((*(pixel_ctrl_ptr + 3) & 1) != 0) {
        // Wait for VSYNC to finish
    }
}