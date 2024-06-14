#include "RTP.h"

void rtpHeaderInit(struct RtpPacket* rtpHeader, uint8_t csrcLen, uint8_t extend,
	uint8_t padding, uint8_t version, uint8_t payloadType, uint8_t mark,
	uint16_t seq, uint32_t timestamp, uint32_t ssrc)
{
	rtpHeader->rtpHeader.csrcLen = csrcLen;
	rtpHeader->rtpHeader.extend = extend;
	rtpHeader->rtpHeader.padding = padding;
	rtpHeader->rtpHeader.version = version;
	rtpHeader->rtpHeader.payloadType = payloadType;
	
	rtpHeader->rtpHeader.mark = mark;
	rtpHeader->rtpHeader.mark = seq;
	rtpHeader->rtpHeader.mark = timestamp;
	rtpHeader->rtpHeader.mark = ssrc;
}

int sendRtpPacketOverTCP(SOCKET tcpSocket, struct RtpPacket* rtpPacket, uint32_t dataSize)
{
	return 0;
}

int sendRtpPacketOverUDP(SOCKET udpSocket, const char* ip, int16_t port, struct RtpPacket* rtpPacket, uint32_t dataSize)
{
	struct sockaddr_in addr;
	int ret;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	rtpPacket->rtpHeader.seq = htons(rtpPacket->rtpHeader.seq);
	rtpPacket->rtpHeader.timestamp = htonl(rtpPacket->rtpHeader.timestamp);
	rtpPacket->rtpHeader.ssrc = htonl(rtpPacket->rtpHeader.ssrc);

	ret = sendto(udpSocket, (char*)rtpPacket, RTP_HEADER_SIZE + dataSize, 0, (struct sockaddr*)&addr, sizeof(addr));

	rtpPacket->rtpHeader.seq = htons(rtpPacket->rtpHeader.seq);
	rtpPacket->rtpHeader.timestamp = htonl(rtpPacket->rtpHeader.timestamp);
	rtpPacket->rtpHeader.ssrc = htonl(rtpPacket->rtpHeader.ssrc);

	return ret;
}