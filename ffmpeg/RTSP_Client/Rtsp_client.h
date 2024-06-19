#pragma once

#include <winsock.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")


class Rtsp_client
{
public:
	int initWinSock();
	int connectServer();

	SOCKET tcpSocket;
};

