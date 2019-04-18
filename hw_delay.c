/*
 * hw_delay.c
 *
 * Created: 11-10-2016 12:23:06
 *  Author: Karol
 */ 


#include "hw_delay.h"
#include <util/atomic.h>

volatile static uint32_t time = 0;

uint32_t getCurrentTime(void){
	uint32_t tmp_time = 0;
	ATOMIC_BLOCK(ATOMIC_FORCEON){
		tmp_time = time;	
	}
	return tmp_time;
}

void hw_delay_ms(uint32_t delay_time){
	//Zerowanie preskalera?
	uint32_t startTime = getCurrentTime();
	while((getCurrentTime() - startTime) < delay_time );
}

void hw_sleep_ms(uint32_t delay_time){
	
	uint32_t startTime = getCurrentTime();
	MCUCR |= (1<<SE);
	while((getCurrentTime() - startTime) < delay_time ){	
		asm volatile("SLEEP");
		asm volatile("NOP");
	};
	MCUCR &= ~(1<<SE);
}

void hw_sleep(uint8_t sleep_mode){
	MCUCR |= (1<<SE) | (sleep_mode << SM0);
	asm volatile("SLEEP");
	asm volatile("NOP");
	MCUCR &= ~((1<<SE) | (1<<SM0) | (1<<SM1) | (1<<SM2));
}

ISR(INT_VECTOR){
	time++;
}