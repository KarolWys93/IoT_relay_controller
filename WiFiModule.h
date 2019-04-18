/*
 * ESP01Module.h
 *
 * Created: 15.03.2019 23:04:23
 *  Author: Karol
 */ 


#ifndef ESP01MODULE_H_
#define ESP01MODULE_H_

#include "defines.h"
#include <avr/io.h>

#define WiFi_init() SET(DDR, WiFi_enable_pin)
#define WiFi_enable() SET(PORT, WiFi_enable_pin)
#define WiFi_disable() CLR(PORT, WiFi_enable_pin)

typedef enum
{
	WiFi_OK = 0,
	WiFi_ALREADY_CONNECT,
	WiFi_ERROR,
	WiFi_NO_IP,
	WiFi_DNS_FAIL

}WiFI_Status;

WiFI_Status WiFi_reset(uint32_t timeout);

WiFI_Status WiFi_SetNetwork(char* SSID, char* password);
WiFI_Status WiFi_checkAPconnection();
WiFI_Status WiFi_openConnection(char* adress, uint16_t port);
WiFI_Status WiFi_closeConnection();
WiFI_Status WiFi_sendData(char* data, uint16_t dataLength);
uint16_t WiFi_readData(char* data, uint16_t bufferLen, uint32_t timeout);


#endif /* ESP01MODULE_H_ */