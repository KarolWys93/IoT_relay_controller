/*
 * WiFiModule.c
 *
 * Created: 15.03.2019 23:40:59
 *  Author: Karol
 */ 

#include "WiFiModule.h"
#include "uart.h"
#include "hw_delay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

//private variables
static char wifi_cmdBuffer[70];

//strings
static const char s_wifi_status_ok[] PROGMEM = "OK";
static const char s_wifi_status_error[] PROGMEM = "ERROR";
static const char s_wifi_status_no_ip[] PROGMEM = "no ip";
static const char s_wifi_status_already_connect[] PROGMEM = "ALREAY CONNECT";
static const char s_wifi_status_dns_fail[] PROGMEM = "DNS Fail";
static const char s_wifi_status_no_ap[] PROGMEM = "No AP";
static const char s_wifi_status_ready[] PROGMEM = "ready";
static const char s_wifi_status_send_ok[] PROGMEM = "SEND OK";

static const char s_wifi_cipstart_cmd[] PROGMEM = "AT+CIPSTART=\"TCP\",\"";
static const char s_wifi_cwjap_set_cmd[] PROGMEM = "AT+CWJAP=\"";
static const char s_wifi_cipsend_cmd[] PROGMEM = "AT+CIPSEND=";

WiFi_Status WiFi_reset(uint32_t timeout){
	WiFi_Status status = WiFi_ERROR;
	sendLine("AT+RST");
	while(1){
		if(readLine(wifi_cmdBuffer, sizeof wifi_cmdBuffer / sizeof *wifi_cmdBuffer, timeout) == 0){break;}
		if(strcmp_P(wifi_cmdBuffer, (char*) &s_wifi_status_ready) == 0){
			status = WiFi_OK;
			break;
		}
	}
	return status;
}

WiFi_Status WiFi_SetNetwork(char* SSID, char* password){
	WiFi_Status status = WiFi_ERROR;

	strcpy_P(wifi_cmdBuffer, (char*) &s_wifi_cwjap_set_cmd);
	strcat(wifi_cmdBuffer, SSID);
	strcat(wifi_cmdBuffer, "\",\"");
    strcat(wifi_cmdBuffer, password);
	strcat(wifi_cmdBuffer, "\"");

	sendLine(wifi_cmdBuffer);
	if(readLine(wifi_cmdBuffer, sizeof wifi_cmdBuffer/sizeof *wifi_cmdBuffer, 10000) == 0){return status;};
	
	if (strcmp_P(wifi_cmdBuffer, (char*) &s_wifi_status_ok) == 0)
	{
		status = WiFi_OK;
	}
	
	memset(wifi_cmdBuffer, 0, sizeof wifi_cmdBuffer/sizeof *wifi_cmdBuffer);
	
	return status;
}

WiFi_Status WiFi_checkAPconnection(){
	WiFi_Status status = WiFi_ERROR;
	bool waitForAnswer = true;

	sendLine("AT+CWJAP?");
	do{
		if(readLine(wifi_cmdBuffer, sizeof wifi_cmdBuffer / sizeof *wifi_cmdBuffer, 10000) == 0){
			break;
		}
		if(strcmp_P(wifi_cmdBuffer, (char*) &s_wifi_status_no_ap) == 0){
			waitForAnswer = false;
		}else if (strcmp_P(wifi_cmdBuffer, (char*) &s_wifi_status_ok) == 0){
			waitForAnswer = false;
			status = WiFi_OK;
		}
	}while(waitForAnswer);

	return status;
}

WiFi_Status WiFi_openConnection(char* adress, uint16_t port){
		WiFi_Status status = WiFi_ERROR;
		bool waitForAnswer = true;
		
		char portAsString[6];
		sprintf(portAsString, "%u", port);
		strcpy_P(wifi_cmdBuffer, (char*)&s_wifi_cipstart_cmd);
		strcat(wifi_cmdBuffer, adress);
		strcat(wifi_cmdBuffer, "\",");
		strcat(wifi_cmdBuffer, portAsString);

		sendLine(wifi_cmdBuffer);
		do{
			if(readLine(wifi_cmdBuffer, sizeof wifi_cmdBuffer / sizeof *wifi_cmdBuffer, 10000) == 0){
				break;
			}
			if(strcmp_P(wifi_cmdBuffer, (char *) &s_wifi_status_ok) == 0){
				waitForAnswer = false;
				status = WiFi_OK;
			}else if(strcmp_P(wifi_cmdBuffer, (char *) &s_wifi_status_error) == 0){
				waitForAnswer = false;
				status = WiFi_ERROR;
			}else if(strcmp_P(wifi_cmdBuffer, (char *) &s_wifi_status_no_ip) == 0){
				waitForAnswer = false;
				status = WiFi_NO_IP;
			}else if(strcmp_P(wifi_cmdBuffer, (char *) &s_wifi_status_already_connect) == 0){
				waitForAnswer = false;
				status = WiFi_ALREADY_CONNECT;
			}else if(strcmp_P(wifi_cmdBuffer, (char *) &s_wifi_status_dns_fail) == 0){
				waitForAnswer = false;
				status = WiFi_DNS_FAIL;
			}
		}while(waitForAnswer);

		return status;
}

WiFi_Status WiFi_closeConnection(){
	WiFi_Status status = WiFi_ERROR;
	bool waitForAnswer = true;

	sendLine("AT+CIPCLOSE");
	do{
		if (readLine(wifi_cmdBuffer, sizeof wifi_cmdBuffer / sizeof *wifi_cmdBuffer, 10000) == 0){
			break;
		}
		if(strcmp_P(wifi_cmdBuffer, (char*)&s_wifi_status_ok) == 0){
			waitForAnswer = false;
			status = WiFi_OK;
		}else if(strcmp_P(wifi_cmdBuffer, (char*)&s_wifi_status_error) == 0){
			waitForAnswer = false;
		}
	}while(waitForAnswer);

	return status;
}

WiFi_Status WiFi_sendData(char* data, uint16_t dataLength){
	WiFi_Status status = WiFi_ERROR;
	bool waitForAnswer = true;
	char sign = '\0';

	sprintf(wifi_cmdBuffer, "%S%d", (wchar_t*)&s_wifi_cipsend_cmd, dataLength);

	sendLine(wifi_cmdBuffer);

	do{
		sign = readChar(10000);
		if(sign == '>'){
			sendData(data, dataLength);
			do{
				if(readLine(wifi_cmdBuffer, sizeof wifi_cmdBuffer / sizeof *wifi_cmdBuffer, 10000) != 0){//TODO add timeout
					if(strcmp_P(wifi_cmdBuffer, (char*) &s_wifi_status_send_ok) == 0){
						status = WiFi_OK;
						waitForAnswer = false;
					}
				}else{
					waitForAnswer = false;
				}
			}while(waitForAnswer);
		}else if(sign == 'l' || sign == '\0'){
			waitForAnswer = false;
		}
	}while(waitForAnswer);

	return status;
}
uint16_t WiFi_readData(char* data, uint16_t bufferLen, uint32_t timeout){
	const char newDataPattern[] = {'+', 'I', 'P', 'D', ','};
	char lengthStr[5];
	uint8_t charCount = 0;
	bool headRecived = false;
	uint16_t length = 0;
	
	uint32_t char_timeout = timeout/4;
	uint32_t startTime = getCurrentTime();
	while(1){

		//step 1: waiting for "+IPD,"
		if(readChar(char_timeout) == newDataPattern[charCount++]){
			if(charCount == 5){
				charCount = 0;
				headRecived = true;
				break;
			}
		}else{
			charCount = 0;
			if((getCurrentTime() - startTime) > timeout){break;}
		}
	}

	if(headRecived){//read number of bytes to receiving
		while(1){
			lengthStr[charCount] = readChar(char_timeout);

			if(lengthStr[charCount] == ':'){
				lengthStr[charCount] = '\0';
				length = atoi(lengthStr);
				break;
			}

			if (++charCount > 5){
				return 0;
			}
		}

		if(length > bufferLen)
		length = bufferLen;
		
		readData(data, length, timeout);
	}

	return length;
}

