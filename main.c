/*
 * relay_controller.c
 *
 * Created: 19.04.2019 01:36:14
 * Author : Karol
 */ 

//buffers size
#define RECIVED_BUFFER_SIZE 64
#define SEND_BUFFER_SIZE 35


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
#include <avr/pgmspace.h>

//strings
static const char s_cipstmac_cmd[] PROGMEM = "AT+CIPSTAMAC?";
static const char s_token[] PROGMEM = "\"temp\"";
static const char s_response_1[] PROGMEM = "{\"state\": 1}";
static const char s_response_0[] PROGMEM = "{\"state\": 0}";	

static uint16_t currentTemp = 150;
static uint16_t setTemp = 150;

static void init(void);
static void work(void);
static WiFi_Status changeWiFiConfig(void);
static void generateDeviceID(void);
static void startWiFi(void);
static void connectToServer(char* sendBuffer, char* recivedBuffer, MqttConfig* mqttConfig, char* deviceID);
static uint8_t startSubscribe(char* sendBuffer, char* recivedBuffer, MqttConfig* mqttConfig);
static uint8_t getTopicType(uint8_t* message, MqttConfig* mqttConfig);
static uint16_t parse2num(char* string, uint8_t len);
static uint16_t parseMessage(uint8_t* message);
static void handleMessage(uint8_t* message, MqttConfig* mqttConfig);



int main(void){
	init();
	config_mode();
	work();
}

static void init(void){
	SET(DDR, RELAY_LINE);
	CLR(PORT, RELAY_LINE);
	SET(PORT, config_mode_button);
	hw_delay_init();
	usartInit();
	WiFi_init();
	WiFi_disable();
	sei();
}

static WiFi_Status changeWiFiConfig(){
	WiFiConfig wifiConfig;
	getWiFiConfig(&wifiConfig);
	WiFi_Status wifiStatus = WiFi_SetNetwork(wifiConfig.ssid, wifiConfig.password);
	memset(wifiConfig.password, 0, WIFI_CONF_BUFFER_SIZE);
	return wifiStatus;
}

static void generateDeviceID(void){
	if (!isDeviceIDok())
	{
		char buffer[32];
		memset(buffer, 0, 32);
		strcpy_P(buffer, (char*)&s_cipstmac_cmd);
		sendLine(buffer);
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

static void startWiFi(void){
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

static uint8_t startSubscribe(char* sendBuffer, char* recivedBuffer, MqttConfig* mqttConfig){
	uint8_t messageLen = 0;
	do 
	{
		messageLen = MQTT_pingPacket((uint8_t*) sendBuffer);
		
		if(WiFi_sendData(sendBuffer, messageLen) != WiFi_OK){
			hw_sleep_ms(1000);
			return 0;
		}
		messageLen = WiFi_readData(recivedBuffer, RECIVED_BUFFER_SIZE, 1000);
		
	} while (messageLen == 0 || MQTT_getType((uint8_t*) recivedBuffer) != MQTT_CTRL_PINGRESP);


	messageLen = MQTT_subscribePacket((uint8_t* )sendBuffer, mqttConfig->status_topic, 0);
	if(WiFi_sendData(sendBuffer, messageLen) != WiFi_OK){return 0;}
		
	if (WiFi_readData(recivedBuffer, RECIVED_BUFFER_SIZE, 2000) != 0)
	{
		uint8_t responseType = MQTT_getType((uint8_t*) recivedBuffer);
		if (responseType == MQTT_CTRL_PUBLISH)
		{
			handleMessage((uint8_t*) recivedBuffer, mqttConfig);
		} else if (responseType != MQTT_CTRL_SUBACK){
			return 0;
		}
	} else {
		return 0;
	}
	
	messageLen = MQTT_subscribePacket((uint8_t* )sendBuffer, mqttConfig->value_topic, 0);
	if(WiFi_sendData(sendBuffer, messageLen) != WiFi_OK){return 0;}

	if (WiFi_readData(recivedBuffer, RECIVED_BUFFER_SIZE, 2000) != 0)
	{
		uint8_t responseType = MQTT_getType((uint8_t*) recivedBuffer);
		if (responseType == MQTT_CTRL_PUBLISH)
		{
			handleMessage((uint8_t*) recivedBuffer, mqttConfig);
		} else if (responseType != MQTT_CTRL_SUBACK){
			return 0;
		}
	} else {
		return 0;
	}
	return 1;
}

static uint8_t getTopicType(uint8_t* message, MqttConfig* mqttConfig){
	if (MQTT_getType(message) == MQTT_CTRL_PUBLISH)
	{
		uint8_t topicLen = MQTT_topicSize(message);
		uint8_t* topic = MQTT_getTopic(message);
		if(strncmp((char* )topic, mqttConfig->status_topic, topicLen) == 0){
			return 1;
		}else if(strncmp((char* )topic, mqttConfig->value_topic, topicLen) == 0){
			return 2;	
		}
	}
	return 0;
}

static uint16_t parse2num(char* string, uint8_t len){
	uint16_t parsedNum = 0;
	uint8_t parsedDecimal = 0;
	uint8_t i;
	for(i = 0; i < len; i++){
		if (string[i] == '.'){
			string[i] = '\0';
			break;
		}
	}
	parsedNum = atoi(string);
	if (i >= len)
	{
		return parsedNum*10;
	}
	
	parsedDecimal = *(string+i+1);
	parsedDecimal -= 48;
	
	if (parsedNum > 99 || parsedDecimal > 9){return 0;}
	return (parsedNum * 10) + parsedDecimal;
}

static uint16_t parseMessage(uint8_t* message){
	uint8_t* payload = MQTT_getPayload(message);
	uint8_t payloadSize = MQTT_payloadSize(message);
	
	payload[payloadSize-1] = '\0';	//the last character should be "}", so we will not lose anything
	
	char* token = strstr_P((char*) payload, (char*) &s_token);	//token size = 6
	if (token == NULL)
	{
		return 0;
	}

	char* startValue = token + 6;
	for (; startValue < ((char*) payload)+payloadSize; startValue++){
		if (*startValue == ':')
		{
			startValue++;
			break;
		}
	}
	
	char* endValue = startValue;
	for (; endValue < ((char*) payload)+payloadSize; endValue++){
		if (*endValue == ',' || *endValue == '}' || *endValue == ']')
		{break;}
	}
	*endValue = '\0';
	return parse2num(startValue, endValue-startValue);
}

static void handleMessage(uint8_t* message, MqttConfig* mqttConfig){
	uint8_t topicType = getTopicType(message, mqttConfig);
	if (topicType == 1 || topicType == 2)
	{
		uint16_t value = parseMessage(message);
		if (value == 0){return;}
		if (topicType == 1){setTemp = value;} else {currentTemp = value;}
	}
}

static void work(void){
	char recivedText[RECIVED_BUFFER_SIZE];
	char sendBuffor[SEND_BUFFER_SIZE];
	uint16_t messageLen = 0;
	startWiFi();

	MqttConfig mqttConfig;
	getMQTTConfig(&mqttConfig);
	
	char deviceID[DEVICE_ID_BUFFER_SIZE];
	getDeviceID(deviceID);
	
	while (1){
		
		connectToServer(sendBuffor, recivedText, &mqttConfig, deviceID);
		
		if(!startSubscribe(sendBuffor, recivedText, &mqttConfig)){
			WiFi_closeConnection();
			continue;
		}
		
		hw_sleep_ms(500);
		
		while(1){
			if (WiFi_readData(recivedText, RECIVED_BUFFER_SIZE, MQTT_CONN_KEEPALIVE * (1000/2)) != 0)
			{	
				handleMessage((uint8_t* )recivedText, &mqttConfig);
				//sprintf(sendBuffor, "V: %d, %d, %d", currentTemp, setTemp, getHysteresis());
				//sendLine(sendBuffor);
				uint8_t lastState = GET(RELAY_LINE) ? 1 : 0;
				uint8_t newState = lastState;
				if (currentTemp <= (setTemp-((10*getHysteresis())/2)))
				{
					if (isInvertMode())
					{
						CLR(PORT, RELAY_LINE);
						newState = 0;
					} else {
						SET(PORT, RELAY_LINE);
						newState = 1;
					}
				} else if (currentTemp >= (setTemp+((10*getHysteresis())/2))){
					if (isInvertMode())
					{
						SET(PORT, RELAY_LINE);
						newState = 1;
					}else{
						CLR(PORT, RELAY_LINE);
						newState = 0;
					}
				}
				
				if (lastState != newState)
				{
					char response[15];
					if (newState)
					{
						strcpy_P(response, (char*)&s_response_1);
					}else{
						strcpy_P(response, (char*)&s_response_0);
					}
					messageLen = MQTT_publishPacket((uint8_t*) sendBuffor, mqttConfig.status_topic, response, 0, 0);
					WiFi_sendData(sendBuffor, messageLen);
				}
			}
			else
			{
				uint8_t attempt = 3;
				do 
				{
					messageLen = MQTT_pingPacket((uint8_t*) sendBuffor);
					if (WiFi_sendData(sendBuffor, messageLen) != 0)
					{
						attempt = 0;
						break;
					}
					messageLen = WiFi_readData(recivedText, RECIVED_BUFFER_SIZE, 1000);
					if ( messageLen == 0 || MQTT_getType((uint8_t*) recivedText) != MQTT_CTRL_PINGRESP)
					{
						attempt--;
						hw_sleep_ms(1000);
					}else{
						break;
					}
				} while (attempt == 0);
				if (attempt == 0){break;}
			}
		}
		WiFi_closeConnection();
		hw_sleep_ms(1000);
	}
}
