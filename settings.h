/*
 * settings.h
 *
 * Created: 14.03.2019 22:18:58
 *  Author: Karol
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <avr/io.h>

#define WIFI_CONF_BUFFER_SIZE 25
#define MQTT_CONF_BUFFER_SIZE 41
#define MQTT_CREDENTIAL_BUFFER_SIZE 25
#define DEVICE_ID_BUFFER_SIZE 9 

typedef struct {
	char host[MQTT_CONF_BUFFER_SIZE];
	char topic[MQTT_CONF_BUFFER_SIZE];
	char mqtt_user[MQTT_CREDENTIAL_BUFFER_SIZE];
	char mqtt_pass[MQTT_CREDENTIAL_BUFFER_SIZE];
	uint16_t port;
} MqttConfig;

typedef struct {
	char ssid[WIFI_CONF_BUFFER_SIZE];
	char password[WIFI_CONF_BUFFER_SIZE];
} WiFiConfig;

/* WIFI config */
uint8_t isWiFiConfigChanged(void);

void getWiFiConfig(WiFiConfig* config);

void setWiFiPassword(char* passwordBuffer, uint8_t len);
void setSSID(char* ssidBuffer, uint8_t len);

/* connection settings */
void getMQTTConfig(MqttConfig* config);

void setHost(char* hostNameBuffer, uint8_t len);
void setPort(uint16_t port);
void setTopic(char* topicBuffer, uint8_t len);
void setMqttUser(char* mqttUserBuffer, uint8_t len);
void setMqttPass(char* mqttPassBuffer, uint8_t len);

/* other settings */
uint16_t getPeriod(void);
void setPeriod(uint16_t period);

uint8_t isDeviceIDok(void);
void setDeviceID(char* deviceID);
void getDeviceID(char* deviceID);

#endif /* SETTINGS_H_ */