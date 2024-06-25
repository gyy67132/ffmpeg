#pragma once

#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

#include "Sdp.h"

struct RtpContext
{
	int playload;//96,97
	const char* encoding;//H264,H265

	FILE* out_f;
	void* decoder;

	size_t size;
	uint8_t packet[64 * 1024];
};

class Rtsp_client
{
	SOCKET tcpSocket;
	char ip[20];
	uint16_t port;
	char mediaRoute[20];
	char userAgent[20];
	char contentBase[100];
	int contentLength;
	char session[20];

	RtpContext *m_rtpCtx;
public:
	Rtsp_client(const char* url);
	~Rtsp_client();
	int initWinSock();
	int connectServer();
	void startCMD();

private:
	int sendCmdOptions(int seq);
	int sendCmdDescribe(int seq);
	int sendCmdSetup(int seq, SdpTrack* track);
	int sendCmdPlay(int seq);
	int sendCmdOverTCP(char* buff, int len);
	void parseData();
	bool parsePacket(char channel, char* packet, int size);
	Sdp sdp;
};

