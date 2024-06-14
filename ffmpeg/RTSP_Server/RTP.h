#pragma once

#include <stdint.h>
#include <winsock2.h>

#define RTP_HEADER_SIZE 12
#define RTP_VERSION 2
#define RTP_PLAYLOAD_TYPE_H264  96
#define RTP_MAX_PKT_SIZE 1420

struct RtpHeader
{
	uint8_t csrcLen : 4;
	uint8_t extend : 1;
	uint8_t padding : 1;
	uint8_t version : 2;//°æ±¾

	uint8_t payloadType : 7;
	uint8_t mark : 1;
	
	uint16_t seq;

	uint32_t timestamp;

	uint32_t ssrc;

	//uint32_t csrc;
};

struct RtpPacket
{
	struct RtpHeader rtpHeader;
	uint8_t payload[0];
};

void rtpHeaderInit(struct RtpPacket* rtpHeader, uint8_t csrcLen, uint8_t extend,
					uint8_t padding, uint8_t version, uint8_t payloadType, uint8_t mark,
					uint16_t seq, uint32_t timestamp, uint32_t ssrc);

int sendRtpPacketOverTCP(SOCKET tcpSocket, struct RtpPacket *rtpPacket, uint32_t dataSize);
int sendRtpPacketOverUDP(SOCKET udpSocket, const char *ip, int16_t port, struct RtpPacket* rtpPacket, uint32_t dataSize);