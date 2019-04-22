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
#include <avr/pgmspace.h>

static const char config_title[] PROGMEM = "Config mode";
static const char config_title_separator[] PROGMEM = "===========";
static const char config_deviceID_label[] PROGMEM = "Device ID:";
static const char config_ssid_label[] PROGMEM = "SSID:";
static const char config_pass_label[] PROGMEM = "Pass:";
static const char config_dummy_password[] PROGMEM = "********";
static const char config_host_label[] PROGMEM = "Host:";
static const char config_vtopic_label[] PROGMEM = "ValueTopic:";
static const char config_stopic_label[] PROGMEM = "StatusTopic:";
static const char config_mqttuser_label[] PROGMEM = "MQTT_User:";
static const char config_mqttpass_label[] PROGMEM = "MQTT_Pass:";

static const char config_confirm_label[] PROGMEM = "ok";
static const char config_promt_label[] PROGMEM = ">";

static const char config_deviceID_cmd[] PROGMEM = "Device ID";
static const char config_ssid_cmd[] PROGMEM = "SSID";
static const char config_pass_cmd[] PROGMEM = "Pass";
static const char config_host_cmd[] PROGMEM = "Host";
static const char config_vtopic_cmd[] PROGMEM = "ValueTopic";
static const char config_stopic_cmd[] PROGMEM = "StatusTopic";
static const char config_mqttuser_cmd[] PROGMEM = "MQTT_User";
static const char config_mqttpass_cmd[] PROGMEM = "MQTT_Pass";
static const char config_port_cmd[] PROGMEM = "Port";
static const char config_hist_cmd[] PROGMEM = "Hist";


static void printFromFlash(const char* text){
	char textBuffer[14];
	strcpy_P(textBuffer, text);
	sendLine(textBuffer);
}

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
		
		printFromFlash((char*) &config_title);
		printFromFlash((char*)&config_title_separator);
		
		printFromFlash((char*)&config_deviceID_label);
		getDeviceID(buffer);
		sendLine(buffer);
		sendLineSeparator();
		
		printFromFlash((char*)&config_ssid_label);
		sendLine(wifiConfig.ssid);
		sendLineSeparator();

		printFromFlash((char*)&config_pass_label);
		printFromFlash((char*)&config_dummy_password);
		sendLineSeparator();

		printFromFlash((char*)&config_host_label);
		sendLine(mqttConfig.host);
		sendLineSeparator();
		
		sprintf(buffer, "%S: %u", (wchar_t* ) &config_port_cmd, mqttConfig.port);
		sendLine(buffer);
		sendLineSeparator();

		printFromFlash((char*)&config_vtopic_label);
		sendLine(mqttConfig.value_topic);
		sendLineSeparator();
		
		printFromFlash((char*)&config_stopic_label);
		sendLine(mqttConfig.status_topic);
		sendLineSeparator();
		
		printFromFlash((char*)&config_mqttuser_label);
		sendLine(mqttConfig.mqtt_user);
		sendLineSeparator();
		
		printFromFlash((char*)&config_mqttpass_label);
		printFromFlash((char*)&config_dummy_password);
		sendLineSeparator();

		sprintf(buffer, "%S: %u", (wchar_t* ) &config_hist_cmd, getHysteresis());
		sendLine(buffer);
		sendLineSeparator();
}

static void getSetting(char *buffer, uint16_t len){
	printFromFlash((char*) &config_promt_label);
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
		 
		 if (strcmp_P(textBuffer, (const char*) &config_ssid_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setSSID(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_pass_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setWiFiPassword(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_host_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setHost(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_port_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setPort(atoi(textBuffer));
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_vtopic_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setValueTopic(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_stopic_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setStatusTopic(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_mqttuser_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setMqttUser(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
				  
		 if (strcmp_P(textBuffer, (const char*) &config_mqttpass_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setMqttPass(textBuffer, BUFFER_SIZE);
			 printFromFlash((char*)&config_confirm_label);
		 }
		 
		 if (strcmp_P(textBuffer, (const char*) &config_hist_cmd) == 0)
		 {
			 getSetting(textBuffer, BUFFER_SIZE);
			 setHysteresis(atoi(textBuffer));
			 printFromFlash((char*)&config_confirm_label);
		 }
	 }
	 #undef BUFFER_SIZE
}