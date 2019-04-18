/*
 * MQTT_client.h
 *
 * Created: 01.04.2019 22:31:42
 *  Author: Karol
 */ 


#ifndef MQTT_CLIENT_H_
#define MQTT_CLIENT_H_
#include <avr/io.h>

#define MQTT_PROTOCOL_LEVEL		4

#define MQTT_CTRL_CONNECT		0x1
#define MQTT_CTRL_CONNECTACK	0x2
#define MQTT_CTRL_PUBLISH		0x3
#define MQTT_CTRL_PUBACK		0x4
#define MQTT_CTRL_PUBREC		0x5
#define MQTT_CTRL_PUBREL		0x6
#define MQTT_CTRL_PUBCOMP		0x7
#define MQTT_CTRL_SUBSCRIBE		0x8
#define MQTT_CTRL_SUBACK		0x9
#define MQTT_CTRL_UNSUBSCRIBE	0xA
#define MQTT_CTRL_UNSUBACK		0xB
#define MQTT_CTRL_PINGREQ		0xC
#define MQTT_CTRL_PINGRESP		0xD
#define MQTT_CTRL_DISCONNECT	0xE

#define MQTT_QOS_1				0x1
#define MQTT_QOS_0				0x0

/* Adjust as necessary, in seconds */
#define MQTT_CONN_KEEPALIVE		60

#define MQTT_CONN_USERNAMEFLAG	0x80
#define MQTT_CONN_PASSWORDFLAG	0x40
#define MQTT_CONN_WILLRETAIN	0x20
#define MQTT_CONN_WILLQOS_1		0x08
#define MQTT_CONN_WILLQOS_2		0x18
#define MQTT_CONN_WILLFLAG		0x04
#define MQTT_CONN_CLEANSESSION	0x02


uint16_t MQTT_connectpacket(uint8_t* packet, char* clientID, char* user, char* pass);

uint16_t MQTT_publishPacket(uint8_t *packet, const char *topic, char *data, uint8_t qos, uint8_t retain);

uint16_t MQTT_subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);

uint16_t MQTT_pingPacket(uint8_t *packet);

uint16_t MQTT_pubackPacket(uint8_t *packet, uint16_t packetID);

//message handling methods
uint8_t MQTT_getType(uint8_t* message);
uint16_t MQTT_getPacketID(uint8_t* message);
uint8_t* MQTT_getPayload(uint8_t* message);
uint8_t MQTT_payloadSize(uint8_t* message);
uint8_t MQTT_getHeaderFlags(uint8_t* message);
uint8_t* MQTT_getTopic(uint8_t* message);
uint8_t MQTT_topicSize(uint8_t* message);

uint8_t MQTT_connackFlag(uint8_t* message);
uint8_t MQTT_connackReturnCode(uint8_t* message);

#endif /* MQTT_CLIENT_H_ */