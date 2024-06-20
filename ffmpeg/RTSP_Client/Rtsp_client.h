#pragma once

#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")


class Rtsp_client
{
	SOCKET tcpSocket;
	char ip[20];
	uint16_t port;
	char mediaRoute[20];
	char userAgent[20];
	char contentBase[100];
	int contentLength;
public:
	Rtsp_client(const char* url);
	int initWinSock();
	int connectServer();
	void startCMD();

private:
	int sendCmdOptions(int seq);
	int sendCmdDescribe(int seq);
	int sendCmdSetup(int seq);
	int sendCmdOverTCP(char* buff, int len);
};

