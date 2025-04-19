#ifndef SCREEN_AND_SOUND_H
#define SCREEN_AND_SOUND_H
#include <stdint.h>

extern const short int startScreen[240][320]; 
extern const short int endScreen[240][320];
extern const short int player1[45][188];
extern const short int player2[45][191];
extern const short int one[77][40];
extern const short int two[77][40];
extern const short int tie[240][320];

extern const int32_t vicSound[28032];
extern const int32_t failSound[32392];

#endif