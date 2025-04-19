#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
	
#define NUM_PAIRS 16
#define TOTAL_CARDS (NUM_PAIRS * 2)
#define HEX_BASE 0xFF200020          // Address for HEX0-HEX3

volatile int pixel_buffer_start; // VGA buffer pointer
volatile int *hex_display = (int *)HEX_BASE; // hex display pointer
volatile int *ps2_ptr; // PS2 keyboard pointer
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
int boxx[8];
int boxy[4];
short int cardDeck [TOTAL_CARDS];
int selectedCardIdx [2] = {-1, -1};
short int allColours[16] = {0xf800, 0x07e0, 0x001f, 0xffe0, 0x07ff,
						   0xf81f, 0xfc00, 0x7bef, 0x780f, 0x04fa,
						   0x87e0, 0xa145, 0xfea0, 0xc618, 0x0f10, 
						   0x0320};//set colours for cards
const int seven_seg_digits[10] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x67   // 9
};
// Keep Track of the scores
int score1 = 0;
int score2 = 0;

int turn1 = 1;
int turn2 = 0;

void waitForVSync();
void clear_screen();
void drawBox(int x, int y, short int boxColor);
void shuffle(short int *array, int size);
int readPS2();
void addBoarder(int selectedIdx, short int boxColor);
void removeBoarder(int selectedIdx, short int boxColor);
int main(void)
{	
	int counter = 0;
  	int counter2 = 0;
	//VGA Display Address
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	// PS/2 Keyboard Address
	ps2_ptr = (int *) 0xFF200100; 
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
	
	
	waitForVSync(); // swap front and back buffers on VGA vertical sync
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer (stores previous buffer1 content)
	
	//read for input and check for correct matching
	int selectedIdx = 0;
	int selectedPrev = 0;	//for remove the previous boarder
	int numSelected = 0;
	short int comparePair[2] = {-1, -1};
	
    while (1)
    {	
		int hex_val = *hex_display;  // Read current HEX display values

		if (turn1) {
		  printf("Player 1's turn!\n");
		} else {
		  printf("Player 2's turn!\n");
		}
		//draw screen
		int flipedCards = 0;
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 4; j++){
				int cardIndex = i * 4 + j; // Ensure each card gets a unique color
				drawBox(boxx[i], boxy[j], cardDeck[cardIndex]);
				if(cardDeck[cardIndex] == 0x0){
					flipedCards++;
				}
			}
		}
		//if all cards are black(fliped) the game end
		if(flipedCards == TOTAL_CARDS){
			break;
		}
	
        //take keyboard input
		int key = readPS2();	
		//w -- 0x1d; a -- 0x1c; s -- 0x1b; d -- 0x23; space -- 0x29
		switch(key){
			case 0x1d: 
				if(selectedIdx % 4 != 0){
					selectedPrev = selectedIdx;
					selectedIdx -= 1;
				}
				break;
			case 0x1c:
				if(selectedIdx >= 4){
					selectedPrev = selectedIdx;
					selectedIdx -= 4;
				}
				break;
			case 0x1b:
				if((selectedIdx+1) % 4 != 0){
					selectedPrev = selectedIdx;
					selectedIdx += 1;
				}
				break;
			case 0x23: 
				if(selectedIdx <= TOTAL_CARDS - 5){
					//-5 to avoid move to the black area somehow
					selectedPrev = selectedIdx;
					selectedIdx += 4;
				}
				break;
			case 0x29:
				/*if selected card below 2:
					record selected index and wait for the next selection
				if selected card above 2:
					reveal and compare cards
				*/
				numSelected ++;
				if(numSelected < 3){
					/*make the selections: 
						can only select card that is not black (not fliped)
						can only select two different card
					*/
					if(cardDeck[selectedIdx] != 0x0){
						if(selectedCardIdx[0] == -1){
							selectedCardIdx[0] = selectedIdx;
						}else if(selectedCardIdx[0] != selectedIdx){
							selectedCardIdx[1] = selectedIdx;
						}else{
							printf("select another card\n");
						}
					}else{
						printf("select a valid card\n");
					}
					
				}else{
					//reveal the card
					addBoarder(selectedCardIdx[0], cardDeck[selectedCardIdx[0]]);
					addBoarder(selectedCardIdx[1], cardDeck[selectedCardIdx[1]]);
					//compare the cards
					if(cardDeck[selectedCardIdx[0]] == cardDeck[selectedCardIdx[1]]){
						//the cards are the same color
						cardDeck[selectedCardIdx[0]] = 0x0;
						cardDeck[selectedCardIdx[1]] = 0x0;
							//reset the selectedcard array and numSelected
						selectedCardIdx[0] = -1;
						selectedCardIdx[0] = -1;
						numSelected = 0;
							//add one mark for the player
						if(turn1){
							score1++;
						}else{
							score2++;
						}
					}else{
						//the cards are not the same
						
							//reset the selectedcard array and numSelected
						selectedCardIdx[0] = -1;
						selectedCardIdx[0] = -1;
						numSelected = 0;
					}
					//update turns
					if (turn1) {
					  turn1 = 0;
					  turn2 = 1;
					} else {
					  turn1 = 1;
					  turn2 = 0;
					}
				}
				
				
				break;
		}
		//remove boarder for previous selected if selection has changed
		removeBoarder(selectedPrev, cardDeck[selectedPrev]);
		//add a yellow boarder for selected cards
		addBoarder(selectedIdx, cardDeck[selectedIdx]);
		if(selectedCardIdx[0] != -1){
			addBoarder(selectedCardIdx[0], cardDeck[selectedCardIdx[0]]);
		}
		
        waitForVSync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		
		
		//update the scores
		*hex_display = 
			(seven_seg_digits[score1]) | (seven_seg_digits[score2] << 16);
    }
	
	//check the scores
	if (score1 > score2) {
		printf("Player 1 won with a score of %d!\n", score1);
	} else if (score2 > score1) {
		printf("Player 2 won with a score of %d!\n", score2);
	} else {
		printf("Its a tie!\n");
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
	for(int i = 0; i < 35; i++){
		for(int j = 0; j < 50; j++){
			int drawx = x + i;
			int drawy = y + j;
			volatile short int *addr = pixel_buffer_start 
			+ (drawy << 10) + (drawx << 1);
			*addr = boxColor;
		}
	}
}

//draw a card with yellow boarder
void addBoarder(int selectedIdx, short int boxColor){
	//calculate the top left corner of the card
	int x = selectedIdx / 4;
	int y = selectedIdx - 4*x;
	for(int i = 0; i < 35; i++){
		for(int j = 0; j < 50; j++){
			int drawx = boxx[x] + i;
			int drawy = boxy[y] + j;
			volatile short int *addr = pixel_buffer_start 
			+ (drawy << 10) + (drawx << 1);
			//add a 5 pixel boarder
			if(i < 5 || i > 30 || j < 5 || j > 45){
				*addr = 0xffe0;
			}else{
				*addr = boxColor;
			}
		}
	}
}

//remove previous selected card's boarder
void removeBoarder(int selectedIdx, short int boxColor){
	int x = selectedIdx / 4;
	int y = selectedIdx - 4*x;
	int boxlocx = boxx[x];
	int boxlocy = boxy[y];
	drawBox(boxlocx, boxlocy, boxColor);
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

//read PS2 input 
int readPS2() {
    int data = *(ps2_ptr); // Read data register
    if (data & 0x8000) { // Check if data is valid
        return data & 0xFF; // Return scan code
    }
    return -1; // No new data
} 
