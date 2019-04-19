/*
 * relay_controller.c
 *
 * Created: 19.04.2019 01:36:14
 * Author : Karol
 */ 

//buffers size
#define RECIVED_BUFFER_SIZE 16
#define SEND_BUFFER_SIZE 64


#include <avr/io.h>
#include <avr/interrupt.h>
#include "defines.h"
#include "uart.h"
#include "settings.h"
#include "hw_delay.h"
#include "WiFiModule.h"
#include "single_bit_io.h"
#include "config_mode.h"
#include "MQTT_client.h"
#include "rand8bit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void init(void);
static void work(void);
static WiFI_Status changeWiFiConfig();

int main(void){
	init();
	config_mode();
	work();
}

static void init(void){
	SET(PORT, config_mode_button);
	hw_delay_init();
	usartInit();
	WiFi_init();
	WiFi_disable();
	sei();
}

static WiFI_Status changeWiFiConfig(){
	WiFiConfig wifiConfig;
	getWiFiConfig(&wifiConfig);
	return WiFi_SetNetwork(wifiConfig.ssid, wifiConfig.password);
}

static void generateDeviceID(void){
	if (!isDeviceIDok())
	{
		char buffer[32];
		memset(buffer, 0, 32);
		sendLine("AT+CIPSTAMAC?");
		do{
			readLine(buffer, 32, 5000);
		}while(buffer[0] != '+' || buffer[2] != 'I');	//1 and 3 letter of +CIPSTAMAC:
		
		uint8_t mac_sum = 0;
		for (uint8_t i = 0; i < strlen(buffer); i++)
		{
			mac_sum += buffer[i];
		}
		setSeed(mac_sum);
		for(uint8_t i = 0; i < DEVICE_ID_BUFFER_SIZE-2; i++)
		{
			uint8_t sign = (generateRand())%62;
			if (sign < 10) {sign+=48;}
			else if(sign >= 10 && sign < 36) {sign+=55;}
			else {sign+=61;}
			buffer[i] = sign;
		}
		buffer[DEVICE_ID_BUFFER_SIZE-1] = 0;
		setDeviceID(buffer);
	}
}

static void startWiFi(){
	WiFi_enable();
	hw_sleep_ms(500);
	WiFi_reset(5000);
	generateDeviceID();
	if (isWiFiConfigChanged())
	{
		changeWiFiConfig();
		hw_sleep_ms(10000);
	}
}

static void connectToServer(char* sendBuffer, char* recivedBuffer, MqttConfig* mqttConfig, char* deviceID){
	uint16_t len = 0;
	while(1){
		while(WiFi_checkAPconnection() != WiFi_OK){
			hw_sleep_ms(5000);
		}
		
		len = MQTT_connectpacket((uint8_t *)sendBuffer, deviceID, mqttConfig->mqtt_user, mqttConfig->mqtt_pass);
		
		while(WiFi_openConnection(mqttConfig->host, mqttConfig->port) != WiFi_OK){
			WiFi_closeConnection();
			hw_sleep_ms(1000);
		}
		
		WiFi_sendData(sendBuffer, len);
		len = WiFi_readData(recivedBuffer, RECIVED_BUFFER_SIZE, 5000);
		if (len > 0 && MQTT_getType((uint8_t*)recivedBuffer) == MQTT_CTRL_CONNECTACK &&	MQTT_connackReturnCode((uint8_t*)recivedBuffer) == 0)
		{
			return;
			} else {
			WiFi_closeConnection();
			hw_sleep_ms(5000);
		}
	}
}

static void work(void){
	char recivedText[RECIVED_BUFFER_SIZE];
	char sendBuffor[SEND_BUFFER_SIZE];
	
	startWiFi();
	
	MqttConfig mqttConfig;
	getMQTTConfig(&mqttConfig);
	
	char deviceID[DEVICE_ID_BUFFER_SIZE];
	getDeviceID(deviceID);
	
	while (1){
		connectToServer(sendBuffor, recivedText, &mqttConfig, deviceID);
		
		WiFi_sendData(sendBuffor, 2);
		WiFi_closeConnection();
	}
}

