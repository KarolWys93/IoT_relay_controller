/*
 * hw_delay.h
 *
 * Created: 11-10-2016 12:03:07
 *  Author: Karol
 */ 


#ifndef HW_DELAY_H_
#define HW_DELAY_H_

#include <avr/interrupt.h>
#include <avr/io.h>

#define sleep_Idle 0
#define sleep_ADC_Noise_Reduction 1
#define sleep_Power_Down 2
#define sleep_Power_Save 3
#define sleep_Standby 6
#define sleep_ExStandby 7

//Type of interruption used by hw_delay
#define INT_VECTOR TIMER2_COMP_vect

inline void hw_delay_init(void){
	//ustawienie licznika 0 na czêstotliwoœæ 1kHz
	// f = fclk/(preskaler*OCR0)
	OCR2 = 125; //przy F_CPU 8MHz -> 125;
	TCCR2 |= (1<<WGM21) | (1<<CS22);	//CTC mode, prescaler 64
	TIMSK |= (1<<OCIE2);
}

uint32_t getCurrentTime(void);

inline uint8_t getRawCurrentTime(void){
	return TCNT2;
}

void hw_delay_ms(uint32_t delay_time);

void hw_sleep_ms(uint32_t delay_time);

void hw_sleep(uint8_t sleep_mode);


#endif /* HW_DELAY_H_ */