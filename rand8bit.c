/*
 * rand8bit.c
 *
 * Created: 13.04.2019 13:06:05
 *  Author: Karol
 */ 

#include "rand8bit.h"

#define a 5
#define c 231

static uint8_t rand_seed;

void setSeed(uint8_t seed){
	rand_seed = seed;
}

uint8_t getSeed(void){
	return rand_seed;
}

uint8_t generateRand(void){
	uint8_t randNum = a*rand_seed + c;
	rand_seed = randNum;
	return randNum;
}