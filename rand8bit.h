/*
 * rand8bit.h
 *
 * Created: 13.04.2019 13:04:04
 *  Author: Karol
 */ 


#ifndef RAND8BIT_H_
#define RAND8BIT_H_


#include <avr/io.h>

void setSeed(uint8_t seed);
uint8_t getSeed(void);
uint8_t generateRand(void);

#endif /* RAND8BIT_H_ */