/*
 * MQTT_client.c
 *
 * Created: 29.03.2019 21:09:34
 *  Author: Karol
 */ 

#include "MQTT_client.h"

#include <string.h>

int16_t packet_id_counter = 0;

char will_topic[] = "";
char will_payload[] ="";
uint8_t will_qos = 1;
uint8_t will_retain = 0;

static uint8_t* AddStringToBuf(uint8_t *_buf, const char *_string)
{
	uint16_t _length = strlen(_string);
	_buf[0] = _length >> 8;
	_buf[1] = _length & 0xFF;
	_buf+=2;
	strncpy((char *)_buf, _string, _length);
	return _buf + _length;
}

uint16_t MQTT_connectpacket(uint8_t* packet, char* clientID, char* user, char* pass)
{
	uint8_t*_packet = packet;
	uint16_t _length;

	_packet[0] = (MQTT_CTRL_CONNECT << 4);
	_packet+=2;
	_packet = AddStringToBuf(_packet, "MQTT");
	_packet[0] = MQTT_PROTOCOL_LEVEL;
	_packet++;
	
	_packet[0] = MQTT_CONN_CLEANSESSION;
	if (will_topic && strlen(will_topic) != 0) {
		_packet[0] |= MQTT_CONN_WILLFLAG;
		if(will_qos == 1)
		_packet[0] |= MQTT_CONN_WILLQOS_1;
		else if(will_qos == 2)
		_packet[0] |= MQTT_CONN_WILLQOS_2;
		if(will_retain == 1)
		_packet[0] |= MQTT_CONN_WILLRETAIN;
	}
	if (strlen(user) != 0)
	_packet[0] |= MQTT_CONN_USERNAMEFLAG;
	if (strlen(pass) != 0)
	_packet[0] |= MQTT_CONN_PASSWORDFLAG;
	_packet++;

	_packet[0] = MQTT_CONN_KEEPALIVE >> 8;
	_packet++;
	_packet[0] = MQTT_CONN_KEEPALIVE & 0xFF;
	_packet++;

	if (strlen(clientID) != 0) {
		_packet = AddStringToBuf(_packet, clientID);
		} else {
		_packet[0] = 0x0;
		_packet++;
		_packet[0] = 0x0;
		_packet++;
	}
	if (will_topic && strlen(will_topic) != 0) {
		_packet = AddStringToBuf(_packet, will_topic);
		_packet = AddStringToBuf(_packet, will_payload);
	}

	if (strlen(user) != 0) {
		_packet = AddStringToBuf(_packet, user);
	}
	
	if (strlen(pass) != 0) {
		_packet = AddStringToBuf(_packet, pass);
	}
	_length = _packet - packet;
	packet[1] = _length-2;

	return _length;
}

uint16_t MQTT_publishPacket(uint8_t *packet, const char *topic, char *data, uint8_t qos, uint8_t retain)
{
	if(retain != 0){retain = 1;}
	uint8_t *_packet = packet;
	uint16_t _length = 0;
	uint16_t Datalen=strlen(data);

	_length += 2;
	_length += strlen(topic);
	if(qos > 0) {
		_length += 2;
	}
	_length += Datalen;
	_packet[0] = MQTT_CTRL_PUBLISH << 4 | qos << 1 | retain << 0;
	_packet++;
	do {
		uint8_t encodedByte = _length % 128;
		_length /= 128;
		if ( _length > 0 ) {
			encodedByte |= 0x80;
		}
		_packet[0] = encodedByte;
		_packet++;
	} while ( _length > 0 );
	_packet = AddStringToBuf(_packet, topic);
	if(qos > 0) {
		_packet[0] = (packet_id_counter >> 8) & 0xFF;
		_packet[1] = packet_id_counter & 0xFF;
		_packet+=2;
		packet_id_counter++;
	}
	memmove(_packet, data, Datalen);
	_packet+= Datalen;
	_length = _packet - packet;

	return _length;
}

uint16_t MQTT_subscribePacket(uint8_t *packet, const char *topic, uint8_t qos)
{
	uint8_t *_packet = packet;
	uint16_t _length;

	_packet[0] = MQTT_CTRL_SUBSCRIBE << 4 | MQTT_QOS_1 << 1;
	_packet+=2;

	_packet[0] = (packet_id_counter >> 8) & 0xFF;
	_packet[1] = packet_id_counter & 0xFF;
	_packet+=2;
	packet_id_counter++;

	_packet = AddStringToBuf(_packet, topic);

	_packet[0] = qos;
	_packet++;

	_length = _packet - packet;
	packet[1] = _length-2;

	return _length;
}

uint16_t MQTT_pubackPacket(uint8_t *packet, uint16_t packetID){
	uint8_t* _packet = packet;
	
	_packet[0] = (MQTT_CTRL_PUBACK << 4);
	_packet[1] = 2;
	_packet+=2;
	_packet[0] = (packetID >> 8) & 0xFF;
	_packet[1] = packetID & 0xFF;
	_packet+=2;
	return _packet - packet;
}

uint16_t MQTT_pingPacket(uint8_t *packet){
	uint8_t* _packet = packet;
	
	_packet[0] = (MQTT_CTRL_PINGREQ << 4);
	_packet[1] = 0;
	_packet+=2;
	return _packet - packet;
}


uint8_t MQTT_getType(uint8_t* message){
	return message[0] >> 4;
}

uint16_t MQTT_getPacketID(uint8_t* message){
	uint16_t packetID = 0;
	uint8_t type = MQTT_getType(message);
	if (type == MQTT_CTRL_PUBACK || type == MQTT_CTRL_SUBACK)
	{
		packetID = (message[2] << 8);
		packetID = packetID | message[3];
	}else if (type == MQTT_CTRL_PUBLISH)
	{
		uint8_t offset = 4 + MQTT_topicSize(message);
		packetID = (message[offset] << 8);
		packetID = packetID | message[offset+1];
	}
	return packetID;	
}

uint8_t* MQTT_getPayload(uint8_t* message){
	uint8_t type = MQTT_getType(message);
	if (type == MQTT_CTRL_PUBLISH)
	{
		return message + 6 + MQTT_topicSize(message);
	} else if(type == MQTT_CTRL_SUBACK){
		return message + 4;	
	} else{
		return 0;
	}		
}

uint8_t MQTT_payloadSize(uint8_t* message){
	uint8_t* payloadPtr = MQTT_getPayload(message);
	uint8_t remainingLength = message[1];
	if (remainingLength > 127){return 0;}

	message += (2 + remainingLength);
	return message - payloadPtr;
}
uint8_t MQTT_getHeaderFlags(uint8_t* message){
	return message[0] & 0x0F;
}

uint8_t* MQTT_getTopic(uint8_t* message){
	if (MQTT_getType(message) == MQTT_CTRL_PUBLISH)
	{
		return message + 4;
	}else{
		return 0;
	}
}

uint8_t MQTT_topicSize(uint8_t* message){
	if (MQTT_getType(message) == MQTT_CTRL_PUBLISH)
	{
		return message[3];	
	}else{
		return 0;
	}
}

uint8_t MQTT_connackFlag(uint8_t* message){
	return message[2] & 1;	
}

uint8_t MQTT_connackReturnCode(uint8_t* message){
	return message[3];
}

