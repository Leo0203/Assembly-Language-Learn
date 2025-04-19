#include <stdio.h>
#include <stdlib.h>
#include <time.h>
	
#define NUM_PAIRS 16
#define TOTAL_CARDS (NUM_PAIRS * 2)

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
int boxx[8];
int boxy[4];
short int cardDeck [TOTAL_CARDS];
short int allColours[16] = {0xf800, 0x07e0, 0x001f, 0xffe0, 0x07ff,
						   0xf81f, 0xfc00, 0x7bef, 0x780f, 0x04fa,
						   0x87e0, 0xa145, 0xfea0, 0xc618, 0x0f10, 
						   0x0320};//set colours for cards

void waitForVSync();
void clear_screen();
void drawBox(int x, int y, short int boxColor);
void shuffle(short int *array, int size);
int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // initialize location, direction, color of rectangles
	for(int i = 0; i < 8; i++){
		boxx[i] = i*35 + 5;
	}
	for(int i = 0; i < 4; i++){
		boxy[i] = i*55 + 5;
	}
	// Duplicate colors to form pairs
	int index = 0;
    for (int i = 0; i < NUM_PAIRS; i++) {
        cardDeck[index++] = allColours[i];  // First copy
        cardDeck[index++] = allColours[i];  // Second copy
    }

    // shuffle the deck
    srand(time(NULL)); // Seed random generator
    shuffle(cardDeck, TOTAL_CARDS);
	
		for (int i = 0; i < TOTAL_CARDS; i++) {
    printf("%04x ", cardDeck[i]);
    if ((i + 1) % 8 == 0) printf("\n"); // Format output
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
	
	//draw initial screen
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 4; j++){
			int cardIndex = i * 4 + j; // Ensure each card gets a unique color
			drawBox(boxx[i], boxy[j], cardDeck[cardIndex]);
		}
	}
	waitForVSync(); // swap front and back buffers on VGA vertical sync
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer (stores previous buffer1 content)
	
	//read for input and check for correct matching
    while (1)
    {
        
		
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

//poll for next available swap of back and front buffer
void waitForVSync(){
	volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
	*(pixel_ctrl_ptr) = 1;
	while ((*(pixel_ctrl_ptr + 3) & 1) != 0) {
        // Wait for VSYNC to finish
    }
}

//draw a box where (x, y) is the top left corner card size: 40 x 60
void drawBox(int x, int y, short int boxColor){
	for(int i = 0; i < 70; i++){
		for(int j = 0; j < 50; j++){
			int drawx = x + i;
			int drawy = y + j;
			volatile short int *addr = pixel_buffer_start 
			+ (drawy << 10) + (drawx << 1);
			*addr = boxColor;
		}
	}
}

//shuffle the array using Fisher-Yates Algorithm
void shuffle(short int *array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // Swap only if i != j
        if (i != j) {
            short int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
}