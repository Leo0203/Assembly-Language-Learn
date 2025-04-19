#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "screen_and_sound.h"

#define NUM_PAIRS 16
#define TOTAL_CARDS (NUM_PAIRS * 2)
#define HEX_BASE 0xFF200020  // Address for HEX0-HEX3
#define AUDIO_BASE 0xFF203040
#define TIMER_BASE 0xFF202000

// Audio playback interface structure
struct audio_interface {
    volatile unsigned int control;
    volatile unsigned char read_available;
    volatile unsigned char read_left;
    volatile unsigned char write_available;
    volatile unsigned char write_left;
    volatile unsigned int left_data;
    volatile unsigned int right_data;
};

// Pointer to the audio interface hardware registers
struct audio_interface *const audio = (struct audio_interface *)0xff203040;


volatile int pixel_buffer_start;              // VGA buffer pointer
volatile int *hex_display = (int *)HEX_BASE;  // hex display pointer
volatile int *ps2_ptr;                        // PS2 keyboard pointer
// VGA Display Address
volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
volatile int *timer = (int *)TIMER_BASE;
short int Buffer1[240][512];  // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
int boxx[8];
int boxy[4];
short int cardDeck[TOTAL_CARDS];
int selectedCardIdx[2] = {-1, -1};
int selectedCardIdxPrev[2] = {-1, -1};
short int
    allColours[16] = {0xf800, 0x07e0, 0x001f, 0xffe0, 0x07ff, 0xf81f,
                      0xfc00, 0x7bef, 0x780f, 0x04fa, 0x87e0, 0xa145,
                      0xfea0, 0xc618, 0x0f10, 0x0320};  // set colours for cards

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

int selectedIdx = 0;

int gameStarted = 0;

int selectedBothCards = 0;

int selectedOneCard = 0;

void waitForVSync();
void clear_screen();
void drawBox(int x, int y, short int boxColor);
void shuffle(short int *array, int size);
char readPS2();
void addBoarder(int selectedIdx, short int boxColor);
void removeBoarder(int selectedIdx, short int boxColor);
void keyboardDelay();
void whiteScreen();
void plot_pixel(int x, int y, short int line_color);  // plots one pixel
void clearBothBuffers();
void play_audio(int *samples, int n, int step, int replicate);
void gameStart();

void initializeGame();

void startTimer();

int main(void) {
  int counter = 0;
  int counter2 = 0;

  // PS/2 Keyboard Address
  ps2_ptr = (int *)0xFF200100;
  // reset the pointer
  *ps2_ptr = 0xFF;

  /* set front pixel buffer to Buffer 1 */
  *(pixel_ctrl_ptr + 1) =
      (int)&Buffer1;  // first store the address in the  back buffer
  /* now, swap the front/back buffers, to set the front buffer location */
  waitForVSync();
  /* initialize a pointer to the pixel buffer, used by drawing functions */
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();  // pixel_buffer_start points to the pixel buffer

  /* set back pixel buffer to Buffer 2 */
  *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
  clear_screen();  // pixel_buffer_start points to the pixel buffer

  waitForVSync();  // swap front and back buffers on VGA vertical sync
  pixel_buffer_start = *(
      pixel_ctrl_ptr + 1);  // new back buffer (stores previous buffer1 content)

  while (1) {
    initializeGame();
    for (int i = 0; i < 320; ++i) {
      for (int j = 0; j < 240; ++j) {
        plot_pixel(i, j, startScreen[j][i]);
      }
    }
    waitForVSync();
    char key;
    do {
      key = readPS2();
    } while (key != 0x29);
    clearBothBuffers();
    gameStart();

    for (int i = 0; i < 320; ++i) {
      for (int j = 0; j < 240; ++j) {
        plot_pixel(i, j, endScreen[j][i]);
      }
    }
    gameStarted = 0;
    // check the scores
    if (score1 > score2) {
      printf("Player 1 won with a score of %d!\n", score1);
      for(int i = 0; i < 40; ++i) {
        for (int j = 0; j < 77; ++j) {
            plot_pixel(i + 270, j + 15, one[j][i]);
        }
      }
    } else if (score2 > score1) {
      printf("Player 2 won with a score of %d!\n", score2);
      for(int i = 0; i < 40; ++i) {
        for (int j = 0; j < 77; ++j) {
            plot_pixel(i + 270, j + 15, two[j][i]);
        }
      }
    } else {
      printf("Its a tie!\n");
    }
    waitForVSync();
    do {
      key = readPS2();
    } while (key != 0x29);
    clearBothBuffers();
  }
}

// clear the screen to totally black
void clear_screen() {
  for (int x = 0; x < 320; x++) {
    for (int y = 0; y < 240; y++) {
      volatile short int *addr = pixel_buffer_start + (y << 10) + (x << 1);
      *addr = 0xFFFF;
    }
  }
}

// poll for next available swap of back and front buffer
void waitForVSync() {
  if (gameStarted) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 4; j++) {
        int cardIndex = i * 4 + j;  // Ensure each card gets a unique color
        if (cardDeck[cardIndex] == 0) {
          drawBox(boxx[i], boxy[j], 0x0);
          if (selectedIdx == cardIndex) {
            addBoarder(selectedIdx, 0x0);
          }

        } else {
          if (selectedIdx == selectedCardIdx[1]) {
            addBoarder(selectedIdx, cardDeck[selectedIdx]);
          }
          if (selectedIdx == cardIndex) {
            addBoarder(selectedIdx, 0xFFFF);
          }
        }
      }
    }
  }

  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  *(pixel_ctrl_ptr) = 1;
  while ((*(pixel_ctrl_ptr + 3) & 1) != 0) {
    // Wait for VSYNC to finish
  }
}

// draw a box where (x, y) is the top left corner card size: 40 x 60
void drawBox(int x, int y, short int boxColor) {
  for (int i = 0; i < 35; i++) {
    for (int j = 0; j < 40; j++) {
      int drawx = x + i;
      int drawy = y + j;
      volatile short int *addr =
          pixel_buffer_start + (drawy << 10) + (drawx << 1);
      *addr = boxColor;
    }
  }
}

// draw a card with yellow boarder
void addBoarder(int selectedIdx, short int boxColor) {
  // calculate the top left corner of the card
  int x = selectedIdx / 4;
  int y = selectedIdx - 4 * x;
  for (int i = 0; i < 35; i++) {
    for (int j = 0; j < 40; j++) {
      int drawx = boxx[x] + i;
      int drawy = boxy[y] + j;
      volatile short int *addr =
          pixel_buffer_start + (drawy << 10) + (drawx << 1);
      // add a 5 pixel boarder
      if (i < 5 || i > 30 || j < 5 || j > 35) {
        *addr = 0xffe0;
      } else {
        *addr = boxColor;
      }
    }
  }
}

// remove previous selected card's boarder
void removeBoarder(int selectedIdx, short int boxColor) {
  int x = selectedIdx / 4;
  int y = selectedIdx - 4 * x;
  int boxlocx = boxx[x];
  int boxlocy = boxy[y];
  drawBox(boxlocx, boxlocy, boxColor);
}

// shuffle the array using Fisher-Yates Algorithm
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

// read PS2 input
char readPS2() {
  char key = 0;
  char release = 0;

  while (((*(timer) & 1) == 0) || !gameStarted) {
    int data = *ps2_ptr;
    if (data & 0x8000) {
      char byte = data & 0xFF;

      // If this is a break code (F0), the next byte is a key release
      if (byte == 0xF0) {
        release = 1;
        continue;
      }

      // If it's a make code and no key is currently being held
      if (!release) {
        key = byte;

        // Wait for key to be released (i.e., see 0xF0 + same key)
        while (1) {
          int release_data = *ps2_ptr;
          if (release_data & 0x8000) {
            char release_byte = release_data & 0xFF;
            if (release_byte == 0xF0) {
              while (!(*ps2_ptr & 0x8000));  // wait for released key
              int discard = *ps2_ptr;        // discard released key code
              break;
            }
          }
        }

        return key;  // finally return the pressed key after release
      }

      release = 0;  // reset release flag mayber return for timer
    }
  }
}

void gameStart() {
  // read for input and check for correct matching
  int selectedPrev = -1;  // for remove the previous boarder
  int selected2Prev = -1;
  int numSelected = 0;
  int blankScreen = 0;
  int flippedCards = 0;
  short int comparePair[2] = {-1, -1};

  clearBothBuffers();

  gameStarted = 1;
  // Keep Track of the scores
  score1 = 0;
  score2 = 0;

  turn1 = 1;
  turn2 = 0;

  selectedIdx = 0;

  for (int i = 0; i < 188; ++i) {
    for (int j = 0; j < 45; ++j) {
      plot_pixel(i + 50, j, player1[j][i]);
    }
  }
  waitForVSync();
  for (int i = 0; i < 188; ++i) {
    for (int j = 0; j < 45; ++j) {
      plot_pixel(i + 50, j, player1[j][i]);
    }
  }

  while (flippedCards < TOTAL_CARDS) {
    if (turn1) {
      printf("Player 1's turn!\n");
      waitForVSync();
      for (int i = 0; i < 188; ++i) {
        for (int j = 0; j < 45; ++j) {
          plot_pixel(i + 50, j, player1[j][i]);
        }
      }
      waitForVSync();
    } else {
      printf("Player 2's turn!\n");
      waitForVSync();
      for (int i = 0; i < 188; ++i) {
        for (int j = 0; j < 45; ++j) {
          plot_pixel(i + 50, j, player2[j][i]);
        }
      }
      waitForVSync();
    }
    startTimer();
    selectedBothCards = 0;
    selectedOneCard = 0;
    numSelected = 0;
    while ((*(timer) & 1) == 0) {
      int hex_val = *hex_display;  // Read current HEX display values
      // draw screen
      // if all cards are black(fliped) the game end

      // take keyboard input
      char key = readPS2();

      // w -- 0x1d; a -- 0x1c; s -- 0x1b; d -- 0x23; space -- 0x29
      switch (key) {
        case 0x1d:
          if (selectedIdx % 4 != 0) {
            selected2Prev = selectedPrev;
            selectedPrev = selectedIdx;
            selectedIdx -= 1;
          }
          break;
        case 0x1c:
          if (selectedIdx >= 4) {
            selected2Prev = selectedPrev;
            selectedPrev = selectedIdx;
            selectedIdx -= 4;
          }
          break;
        case 0x1b:
          if ((selectedIdx + 1) % 4 != 0) {
            selected2Prev = selectedPrev;
            selectedPrev = selectedIdx;
            selectedIdx += 1;
          }
          break;
        case 0x23:
          if (selectedIdx <= TOTAL_CARDS - 5) {
            //-5 to avoid move to the black area somehow
            selected2Prev = selectedPrev;
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

          if (numSelected < 3) {
            /*make the selections:
                            can only select card that is not black (not fliped)
                            can only select two different card
            */
            if (cardDeck[selectedIdx] != 0x0) {
              if (selectedCardIdx[0] == -1) {
                selectedCardIdx[0] = selectedIdx;
                numSelected++;
              } else if (selectedCardIdx[0] != selectedIdx) {
                selectedCardIdx[1] = selectedIdx;
                numSelected++;
              } else {
                printf("select another card\n");
              }
            } else {
              printf("select a valid card\n");
            }
          }
          // waitForVSync();
          break;
        default:
          break;
      }
      if (numSelected > 1) {
        removeBoarder(selectedPrev, 0xFFFF);
        addBoarder(selectedCardIdx[0], cardDeck[selectedCardIdx[0]]);
        addBoarder(selectedCardIdx[1], cardDeck[selectedCardIdx[1]]);
        waitForVSync();
        // reveal the card
        //  addBoarder(selectedCardIdx[0], cardDeck[selectedCardIdx[0]]);
        //  addBoarder(selectedCardIdx[1], cardDeck[selectedCardIdx[1]]);
        // compare the cards
        if (cardDeck[selectedCardIdx[0]] == cardDeck[selectedCardIdx[1]]) {
          printf("cards  same\n");
          // the cards are the same color
          cardDeck[selectedCardIdx[0]] = 0x0;
          cardDeck[selectedCardIdx[1]] = 0x0;
          play_audio(vicSound, 28032/2, 1, 1);
          selectedBothCards = 1;
          // int xValue = selectedCardIdx[0]/4;
          // int yValue = selectedCardIdx[0] % 4;
          // drawBox(boxx[xValue], boxy[yValue], 0x0);

          // xValue = selectedCardIdx[1]/4;
          // yValue = selectedCardIdx[1] % 4;
          // drawBox(boxx[xValue], boxy[yValue], 0x0);

          flippedCards += 2;
          // reset the selectedcard array and numSelected
          // selectedCardIdx[0] = -1;
          // selectedCardIdx[0] = -1;
          // numSelected = 0;
          // add one mark for the player
          if (turn1) {
            score1++;
          } else {
            score2++;
          }
        } else {
          // the cards are not the same
          printf("cards not same\n");
          play_audio(failSound, 32392/2, 1, 1);
          selectedBothCards = 1;
          // reset the selectedcard array and numSelected
          // selectedCardIdx[0] = -1;
          // selectedCardIdx[0] = -1;
          // numSelected = 0;
        }
        // update turns
        if (turn1) {
          turn1 = 0;
          turn2 = 1;
        } else {
          turn1 = 1;
          turn2 = 0;
        }
        for (int i = 0; i < 10000000; ++i) {
        }
        if (turn1) {
          for (int i = 0; i < 188; ++i) {
            for (int j = 0; j < 45; ++j) {
              plot_pixel(i + 50, j, player1[j][i]);
            }
          }
        } else {
          for (int i = 0; i < 188; ++i) {
            for (int j = 0; j < 45; ++j) {
              plot_pixel(i + 50, j, player2[j][i]);
            }
          }
        }
        removeBoarder(selectedCardIdx[0], 0xFFFF);
        removeBoarder(selectedCardIdx[1], 0xFFFF);
        printf("clear both selected\n");
        selectedCardIdxPrev[0] = selectedCardIdx[0];
        selectedCardIdxPrev[1] = selectedCardIdx[1];
        selectedCardIdx[0] = -1;
        selectedCardIdx[1] = -1;

        numSelected = 0;
        waitForVSync();
      }
      // remove boarder for previous selected if selection has changed
      if (selected2Prev != -1) {
        removeBoarder(selected2Prev, 0xFFFF);
      }
      if (selectedPrev != -1) {
        removeBoarder(selectedPrev, 0xFFFF);
      }
      removeBoarder(selectedCardIdxPrev[0], 0xFFFF);
      removeBoarder(selectedCardIdxPrev[1], 0xFFFF);
      // add a yellow boarder for selected cards
      addBoarder(selectedIdx, 0xFFFF);
      if (selectedCardIdx[0] != -1) {
        addBoarder(selectedCardIdx[0], 0xFFFF);
      }
      if (selectedCardIdx[1] != -1) {
        addBoarder(selectedCardIdx[1], 0xFFFF);
      }
      if (numSelected > 1) {
      }

      waitForVSync();  // swap front and back buffers on VGA vertical sync
      pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer

      removeBoarder(selectedCardIdxPrev[0], 0xFFFF);
      removeBoarder(selectedCardIdxPrev[1], 0xFFFF);

      if (turn1) {
        for (int i = 0; i < 188; ++i) {
          for (int j = 0; j < 45; ++j) {
            plot_pixel(i + 50, j, player1[j][i]);
          }
        }
      } else {
        for (int i = 0; i < 188; ++i) {
          for (int j = 0; j < 45; ++j) {
            plot_pixel(i + 50, j, player2[j][i]);
          }
        }
      }

      // update the scores
      *hex_display =
          (seven_seg_digits[score1]) | (seven_seg_digits[score2] << 16);
      if (selectedBothCards) {
        break;
      }
    }

    if (!selectedBothCards) {
      for (int i = 0; i < 2; ++i) {
        if (selected2Prev != -1) {
            removeBoarder(selected2Prev, 0xFFFF);
        }
        if (selectedPrev != -1) {
            removeBoarder(selectedPrev, 0xFFFF);
        }
        if (selectedCardIdx[i] != -1) {
          removeBoarder(selectedCardIdx[i], 0xFFFF);
          waitForVSync();
          pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
          if (selected2Prev != -1) {
            removeBoarder(selected2Prev, 0xFFFF);
          }
          if (selectedPrev != -1) {
            removeBoarder(selectedPrev, 0xFFFF);
          }
          removeBoarder(selectedCardIdx[i], 0xFFFF);
          selectedCardIdx[i] = -1;
          selectedOneCard++;
        }
      }

      if (selectedOneCard > 0) {
        waitForVSync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
      } else {
        waitForVSync();
          pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
          if (selected2Prev != -1) {
            removeBoarder(selected2Prev, 0xFFFF);
          }
          if (selectedPrev != -1) {
            removeBoarder(selectedPrev, 0xFFFF);
          }
      }

      if (turn1) {
        turn1 = 0;
        turn2 = 1;
      } else {
        turn1 = 1;
        turn2 = 0;
      }
      if (turn1) {
        for (int i = 0; i < 188; ++i) {
          for (int j = 0; j < 45; ++j) {
            plot_pixel(i + 50, j, player1[j][i]);
          }
        }
      } else {
        for (int i = 0; i < 188; ++i) {
          for (int j = 0; j < 45; ++j) {
            plot_pixel(i + 50, j, player2[j][i]);
          }
        }
      }
            waitForVSync();  // swap front and back buffers on VGA vertical sync
      pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
    }
  }
}

void plot_pixel(int x, int y, short int line_color) {
  volatile short int *one_pixel_address;

  one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

  *one_pixel_address = line_color;
}

void initializeGame() {
  // initialize location, direction, color of rectangles
  for (int i = 0; i < 8; i++) {
    boxx[i] = i * 35 + 5;
  }
  for (int i = 0; i < 4; i++) {
    boxy[i] = i * 45 + 50;
  }
  // Duplicate colors to form pairs
  int index = 0;
  for (int i = 0; i < NUM_PAIRS; i++) {
    cardDeck[index++] = allColours[i];  // First copy
    cardDeck[index++] = allColours[i];  // Second copy
  }

  // shuffle the deck
  srand(time(NULL));  // Seed random generator
  shuffle(cardDeck, TOTAL_CARDS);
}

void clearBothBuffers() {
  // Clear front buffer
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();
  waitForVSync();

  // Clear back buffer
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);
  clear_screen();
}

// Function to play audio
void play_audio(int *samples, int n, int step, int replicate) {
    int i;
    // Clear output FIFOs and resume conversion
    audio->control = 0x8;
    audio->control = 0x0;
    for (i = 0; i < n; i += step) {
        for (int r = 0; r < replicate; r++) {
            // Wait until there is space in the FIFO
            while (audio->write_available == 0);
            // Write the sample to both channels
            audio->left_data = samples[i];
            audio->right_data = samples[i];
        }
    }
}

void startTimer() {
    int timerDelay = 800000000;
    *(timer) = 0;
    *(timer + 2) = timerDelay;
    *(timer + 3) = (timerDelay >> 16);
    *(timer + 1)  = 4;
}