/*
 * uart.h
 *
 * Created: 16-06-2017 16:23:36
 *  Author: Karol
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdbool.h>
#include <avr/io.h>



void usartInit(void);

bool uartTxIsBusy(void);

void sendLine(char *text);
void sendData(char *text, uint8_t len);

char readChar(uint16_t timeout);
uint16_t readLine(char *buffer, uint16_t len, uint16_t timeout);
uint16_t readData(char *buffer, uint16_t len, uint16_t timeout);


#endif /* UART_H_ */