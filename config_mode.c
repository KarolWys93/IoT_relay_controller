/*
 * config_mode.c
 *
 * Created: 26.03.2019 23:44:14
 *  Author: Karol
 */ 

#include "config_mode.h"
#include <avr/io.h>
#include "uart.h"
#include "settings.h"
#include "single_bit_io.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void sendLineSeparator(void){
	sendLine("");
}
static void printConfig(void){
		WiFiConfig wifiConfig;
		MqttConfig mqttConfig;
		getWiFiConfig(&wifiConfig);
		getMQTTConfig(&mqttConfig);
		char buffer[16];
		memset(buffer, 0, 16);
			
		sendLine("Config mode");
		sendLine("===========");
		
		sendLine("Device ID:");
		getDeviceID(buffer);
		sendLine(buffer);
		sendLineSeparator();
		
		sendLine("SSID:");
		sendLine(wifiConfig.ssid);
		sendLineSeparator();

		sendLine("Pass:");
		sendLine("********");
		sendLineSeparator();

		sendLine("Host:");
		sendLine(mqttConfig.host);
		sendLineSeparator();

		sprintf(buffer, "Port: %u", mqttConfig.port);
		sendLine(buffer);
		sendLineSeparator();

		sendLine("Topic:");
		sendLine(mqttConfig.topic);
		sendLineSeparator();
		
		sendLine("MQTT_User:");
		sendLine(mqttConfig.mqtt_user);
		sendLineSeparator();
		
		sendLine("MQTT_Pass:");
		sendLine("********");
		sendLineSeparator();

		sprintf(buffer, "Period: %u", getPeriod());
		sendLine(buffer);
		sendLineSeparator();
}

static void sendOK(void){
	sendLine("ok");
}

static void getSetting(char *buffer, uint16_t len){
	sendLine(">");
	readLine(buffer, len, 0);
}

void config_mode(void){
	
	//if button is release, return from config mode
	if (GET(config_mode_button))
	{
		return;
	}
	
	printConfig();
	
	#define BUFFER_SIZE 67
	char textBuffer[BUFFER_SIZE];
	memset(textBuffer, 0, BUFFER_SIZE);
	
	 while (1)
	 {
		 readLine(textBuffer, BUFFER_SIZE, 0);
		 sendLine(textBuffer);
		 
		 if (strcmp(textBuffer, "SSID") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setSSID(textBuffer, BUFFER_SIZE);
			 sendOK();
		 }
		 
		 if (strcmp(textBuffer, "Pass") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setWiFiPassword(textBuffer, BUFFER_SIZE);
			 sendOK();
		 }
		 
		 if (strcmp(textBuffer, "Host") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setHost(textBuffer, BUFFER_SIZE);
			 sendOK();
		 }
		 
		 if (strcmp(textBuffer, "Port") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setPort(atoi(textBuffer));
			 sendOK();
		 }
		 
		 if (strcmp(textBuffer, "Topic") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setTopic(textBuffer, BUFFER_SIZE);
			 sendOK();
		 }
		 
		 if (strcmp(textBuffer, "MQTT_User") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setMqttUser(textBuffer, BUFFER_SIZE);
			 sendOK();
		 }
				  
		 if (strcmp(textBuffer, "MQTT_Pass") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setMqttPass(textBuffer, BUFFER_SIZE);
			 sendOK();
		 }
		 
		 if (strcmp(textBuffer, "Period") == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setPeriod(atoi(textBuffer));
			 sendOK();
		 }
	 }
	 #undef BUFFER_SIZE
}