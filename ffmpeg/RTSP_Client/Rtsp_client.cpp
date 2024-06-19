#include "Rtsp_client.h"

int Rtsp_client::initWinSock()
{
	WORD wVersionRequeted = MAKEWORD(2, 2);
	WSADATA WSAData;
	int err = WSAStartup(wVersionRequeted, &WSAData);
	if (err != 0) {
		std::cerr << "init error" << std::endl;
		return -1;
	}

	SOCKET tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket < 0)
	{
		std::cerr << "create socket error\n";
		return -1;
	}
	return 0;
}

int Rtsp_client::connectServer()
{
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(554);
	server.sin_family = AF_INET;

	if (connect(tcpSocket, (sockaddr*)&server, sizeof(server)) == -1)
	{
		std::cerr << "connect server error\n";
		return -1;
	}

	return 0;
}