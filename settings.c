/*
 * settings.c
 *
 * Created: 14.03.2019 23:40:57
 *  Author: Karol
 */ 

#include "settings.h"
#include <avr/eeprom.h>


/* Default values */
EEMEM uint8_t configChange = 1;
EEMEM char ssid_eeprom[WIFI_CONF_BUFFER_SIZE] = "wifi_name";
EEMEM char password_eeprom[WIFI_CONF_BUFFER_SIZE] = "";

EEMEM char host_eeprom[MQTT_CONF_BUFFER_SIZE] = "host.example.com";
EEMEM char value_topic_eeprom[MQTT_CONF_BUFFER_SIZE] = "topic/value_example";
EEMEM char status_topic_eeprom[MQTT_CONF_BUFFER_SIZE] = "topic/status_example";
EEMEM char mqtt_user_eeprom[MQTT_CREDENTIAL_BUFFER_SIZE] = "";
EEMEM char mqtt_pass_eeprom[MQTT_CREDENTIAL_BUFFER_SIZE] = "";
EEMEM uint16_t port_eeprom = 1883;

EEMEM uint16_t hysteresis_eeprom = 1;

EEMEM char device_id_eeprom[DEVICE_ID_BUFFER_SIZE] = "--------";
EEMEM uint8_t device_id_ok_eeprom = 0;


/* WIFI config */
static void setConfigChanged(uint8_t value){
	eeprom_write_byte(&configChange, value);
}


uint8_t isWiFiConfigChanged(void){
	uint8_t isChanged = eeprom_read_byte(&configChange);
	if (isChanged != 0)
	{
		setConfigChanged(0);
	}
	return isChanged;
}

void getWiFiConfig(WiFiConfig* config){
	eeprom_read_block(config->ssid, ssid_eeprom, WIFI_CONF_BUFFER_SIZE);
	eeprom_read_block(config->password , password_eeprom, WIFI_CONF_BUFFER_SIZE);
	config->ssid[WIFI_CONF_BUFFER_SIZE - 1] = '\0';
	config->password[WIFI_CONF_BUFFER_SIZE - 1] = '\0';
}

void setWiFiPassword(char* passwordBuffer, uint8_t len){
	if(len > WIFI_CONF_BUFFER_SIZE){
		len = WIFI_CONF_BUFFER_SIZE;
	}
	eeprom_write_block(passwordBuffer, password_eeprom, len);
	setConfigChanged(1);
}
void setSSID(char* ssidBuffer, uint8_t len){
	if(len > WIFI_CONF_BUFFER_SIZE){
		len = WIFI_CONF_BUFFER_SIZE;
	}
	eeprom_write_block(ssidBuffer, ssid_eeprom, len);
	setConfigChanged(1);
}


/* connection settings */
void getMQTTConfig(MqttConfig* config){
	eeprom_read_block(config->host, host_eeprom, MQTT_CONF_BUFFER_SIZE);
	eeprom_read_block(config->value_topic, value_topic_eeprom, MQTT_CONF_BUFFER_SIZE);
	eeprom_read_block(config->status_topic, status_topic_eeprom, MQTT_CONF_BUFFER_SIZE);
	eeprom_read_block(config->mqtt_user, mqtt_user_eeprom, MQTT_CREDENTIAL_BUFFER_SIZE);
	eeprom_read_block(config->mqtt_pass, mqtt_user_eeprom, MQTT_CREDENTIAL_BUFFER_SIZE);
	
	config->port = eeprom_read_word(&port_eeprom);
	
	config->host[MQTT_CONF_BUFFER_SIZE - 1] = '\0';
	config->value_topic[MQTT_CONF_BUFFER_SIZE - 1] = '\0';
	config->status_topic[MQTT_CONF_BUFFER_SIZE - 1] = '\0';
	config->mqtt_user[MQTT_CREDENTIAL_BUFFER_SIZE - 1] = '\0';
	config->mqtt_pass[MQTT_CREDENTIAL_BUFFER_SIZE - 1] = '\0';		
}

void setHost(char* hostNameBuffer, uint8_t len){
		if(len > MQTT_CONF_BUFFER_SIZE){
			len = MQTT_CONF_BUFFER_SIZE;
		}
		eeprom_write_block(hostNameBuffer, host_eeprom, len);
}
void setPort(uint16_t port){
	eeprom_write_word(&port_eeprom, port);
}
void setValueTopic(char* valueTopicBuffer, uint8_t len){
		if(len > MQTT_CONF_BUFFER_SIZE){
			len = MQTT_CONF_BUFFER_SIZE;
		}
		eeprom_write_block(valueTopicBuffer, value_topic_eeprom, len);
}

void setStatusTopic(char* statusTopicBuffer, uint8_t len){
	if(len > MQTT_CONF_BUFFER_SIZE){
		len = MQTT_CONF_BUFFER_SIZE;
	}
	eeprom_write_block(statusTopicBuffer, status_topic_eeprom, len);
}

void setMqttUser(char* mqttUserBuffer, uint8_t len){
	if(len > MQTT_CREDENTIAL_BUFFER_SIZE){
		len = MQTT_CREDENTIAL_BUFFER_SIZE;
	}
	eeprom_write_block(mqttUserBuffer, mqtt_user_eeprom, len);
}

void setMqttPass(char* mqttPassBuffer, uint8_t len){
	if(len > MQTT_CREDENTIAL_BUFFER_SIZE){
		len = MQTT_CREDENTIAL_BUFFER_SIZE;
	}
	eeprom_write_block(mqttPassBuffer, mqtt_pass_eeprom, len);
}

/* other settings */
uint16_t getHysteresis(void){
	return 100*eeprom_read_word(&hysteresis_eeprom);
}
void setHysteresis(uint16_t hysteresis){
	eeprom_write_word(&hysteresis_eeprom, hysteresis);
}

uint8_t isDeviceIDok(void){
	return eeprom_read_byte(&device_id_ok_eeprom);
}

static void setDeviceIDok(uint8_t ok){
	eeprom_write_byte(&device_id_ok_eeprom, ok);
}

void setDeviceID(char* deviceID){
	eeprom_write_block(deviceID, device_id_eeprom, DEVICE_ID_BUFFER_SIZE);
	setDeviceIDok(1);
}

void getDeviceID(char* deviceID){
	eeprom_read_block(deviceID, device_id_eeprom, DEVICE_ID_BUFFER_SIZE);
}
