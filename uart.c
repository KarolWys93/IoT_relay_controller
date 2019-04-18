/*
 * uart.c
 *
 * Created: 16-06-2017 16:23:11
 *  Author: Karol
 */ 

#include "uart.h"
#include "defines.h"
#include "hw_delay.h"
#include <avr/interrupt.h>
#include <stddef.h>
#include <util/atomic.h>
#include <string.h>

static char* toSendData;
volatile static uint8_t toSendLen = 0;
volatile static bool uartTxBusy = false;

volatile static char dumbByte = '\0';
volatile static uint16_t usartRxCounter;
volatile static uint16_t usartRxBuffLen;
static char* recivedBuff;


void usartInit(void)
{
	// ustawienie transmisji
	#include <util/setbaud.h> //linkowanie tego pliku musi byæ
	//po zdefiniowaniu BAUD

	//ustaw obliczone przez #define wartoœci
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |= (1 << U2X);
	#else
	UCSRA &= ~(1 << U2X);
	#endif

	//Ustawiamy pozostale parametry modul USART

	//standardowe parametry transmisji modulu HC-05
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);  //bitów danych: 8
	//bity stopu:  1
	//parzystoœæ:  brak
	//wlacz nadajnik i odbiornik oraz ich przerwania odbiornika
	//przerwania nadajnika wlaczamy w funkcji wyslij_wynik()
	UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE);
}

bool uratTxIsBusy(void){
	return uartTxBusy;
}

void sendLine(char *text){
	sendData(text, strlen(text));
	while(uartTxBusy);
	sendData("\r\n", 2);
}

void sendData(char *text, uint8_t len){
	//Zaczekaj, az bufor nadawania bedzie pusty
	while (!(UCSRA & (1 << UDRE)));
	toSendData = text;
	toSendLen = len;
	uartTxBusy = true;
	//rozpoczêcie transmisji
	UCSRB |= (1 << UDRIE);
}


static void setRxBuffer(char* buffer, uint16_t len){
	ATOMIC_BLOCK(ATOMIC_FORCEON){
		recivedBuff = buffer;
		usartRxBuffLen = len;
		usartRxCounter = 0;
	}
}

uint16_t readLine(char* buffer, uint16_t len, uint16_t timeout){
	uint32_t startTime = getCurrentTime();
	uint16_t end_byte = 0;
	uint16_t start_byte = 0;
	uint8_t receiving = 1;
	
	setRxBuffer(buffer, len);
	while(receiving){
		hw_sleep(sleep_Idle);
		if (timeout > 0 && (getCurrentTime() - startTime) > timeout )
		{
			setRxBuffer(NULL, 0);
			return 0;
		}
		//start_byte = end_byte;
		ATOMIC_BLOCK(ATOMIC_FORCEON){
			end_byte = usartRxCounter;
		}
		
		for (uint16_t i = start_byte; i < end_byte; i++)
		{
			if(recivedBuff[i] == '\n'){
				recivedBuff[i-1] = '\0';
				receiving = 0;
			}
		}
		
		if(end_byte >= usartRxBuffLen-1){
			recivedBuff[usartRxBuffLen-1] = '\0';
			receiving = 0;
		}
	}
	setRxBuffer(NULL, 0);
	return end_byte;
}

char readChar(uint16_t timeout){
	char receivedChar = '\0';
	if (readData(&receivedChar, 1, timeout) == 0)
	{
		return 0;
	}
	return receivedChar;
}

uint16_t readData(char* buffer, uint16_t len, uint16_t timeout){
	uint32_t startTime = getCurrentTime();
	uint16_t receivedBytesCount = 0;
	setRxBuffer(buffer, len);
	while(receivedBytesCount < len ){
		hw_sleep(sleep_Idle);
		if (timeout > 0 && (getCurrentTime() - startTime) > timeout )
		{
			setRxBuffer(NULL, 0);
			return 0;
		}
		ATOMIC_BLOCK(ATOMIC_FORCEON){
			receivedBytesCount = usartRxCounter;
		}
	}
	setRxBuffer(NULL, 0);
	return receivedBytesCount;
}

ISR(USART_UDRE_vect)
{
	//sprawdzamy, wyslany bajt jest znakiem konca tesktu
	if (toSendLen != 0){
		UDR = *toSendData;
		toSendData++;
		toSendLen--;
	}else{
		uartTxBusy = false;
		UCSRB &= ~(1 << UDRIE); //wylacz przerwania pustego bufora nadawania
	}
}

ISR(USART_RXC_vect)
{
	if (usartRxCounter >= usartRxBuffLen)
	{
		dumbByte = UDR;
		return;
	}
	
	recivedBuff[usartRxCounter++] = UDR;	
}