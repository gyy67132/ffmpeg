#include "Rtsp_client.h"

using namespace std;

Rtsp_client::Rtsp_client(const char *url)
{
	//"rtsp://127.0.0.1:554/live/test";
	if (strncmp(url, "rtsp://", 7) != 0)
	{
		std::cerr << "url error" << std::endl;
	}

	if (sscanf(url + 7, "%[^:]:%hu/%s", ip, &port, mediaRoute) != 3)
	{
		std::cerr << "parse error" << std::endl;
	}

	strcpy(userAgent, "GGY_RtspClient");
}

int Rtsp_client::initWinSock()
{
	WORD wVersionRequeted = MAKEWORD(2, 2);
	WSADATA WSAData;
	int err = WSAStartup(wVersionRequeted, &WSAData);
	if (err != 0) {
		std::cerr << "init error" << std::endl;
		return -1;
	}

	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket < 0)
	{
		std::cerr << "create socket error\n";
		return -1;
	}
	return 0;
}

int Rtsp_client::connectServer()
{
	struct sockaddr_in client;
	client.sin_addr.S_un.S_addr = INADDR_ANY;
	client.sin_family = AF_INET;

	if (bind(tcpSocket, (sockaddr*)&client, sizeof(sockaddr)) == -1)
	{
		std::cerr << "bind error\n";
		return -1;
	}

	struct sockaddr_in server;
	server.sin_port = htons(554);
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &server.sin_addr);
	if (connect(tcpSocket, (sockaddr*)&server, sizeof(server)) == -1)
	{
		std::cerr << "connect server error\n";
		return -1;
	}

	return 0;
}

void Rtsp_client::startCMD()
{
	int recvSeq = 0;
	int sendSeq = 1;
	if (sendCmdOptions(sendSeq) <= 0)
	{
		std::cerr << "sendCmdOptions error\n";
		return;
	}

	const int buffSize = 1024;
	char recvBuf[buffSize];
	int responseStateCode;
	

	while (1)
	{
		int ret = recv(tcpSocket, recvBuf, buffSize, 0);
		if (ret <= 0)
		{
			std::cerr << "recv error\n";
			return;
		}
		recvBuf[ret] = '\0';
		std::cout << "recv-------------------" << endl;
		std::cout << recvBuf << endl;

		responseStateCode = 0;
		recvSeq = 0;

		char* line = strtok(recvBuf, "\n");
		while (line)
		{
			if (strstr(line, "RTSP/1.0")) {
				if (sscanf(line, "RTSP/1.0 %d OK", &responseStateCode) != 1) {
					std::cerr << "parse RTSP/1.0 error\n";
					goto FINISH;
				}
			}
			else if (strstr(line, "CSeq:"))
			{
				if (sscanf(line, "CSeq: %d", &recvSeq) != 1) {
					std::cerr << "parse seq error\n";
					goto FINISH;
				}
			}
			else if (strstr(line, "Content-Base")) {
				if (sscanf(line, "Content-Base: %s\r\n", contentBase) != 1)
				{
					std::cerr << "parse Content-Base error\n";
					goto FINISH;
				}
			}else if (strstr(line, "Content-Length")) {
				if (sscanf(line, "Content-Length: %d\r\n", &contentLength) != 1)
				{
					std::cerr << "parse Content-Length error\n";
					goto FINISH;
				}
			}

			line = strtok(NULL, "\n");
		}

		if (responseStateCode == 200)
		{
			if (recvSeq == 1)
			{
				if (sendCmdDescribe(++sendSeq) < 0)
				{
					std::cerr << "sendCmdDescribe error\n";
					goto FINISH;
				}
			}
			else if (recvSeq == 2)
			{
				if (sendCmdSetup(++sendSeq) < 0)
				{
					std::cerr << "sendCmdSetup error\n";
					goto FINISH;
				}
			}
		}
		else {
			std::cerr << "responseStateCode error " << responseStateCode << endl;
			goto FINISH;
		}
	}

FINISH:
	return;
}

int Rtsp_client::sendCmdOptions(int seq)
{
	char buf[200];
	sprintf(buf, "OPTIONS rtsp://%s:%d/%s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"User-Agent: %s\r\n"
		"\r\n", ip, port, mediaRoute, seq, userAgent);
	return sendCmdOverTCP(buf, strlen(buf));
}

int Rtsp_client::sendCmdDescribe(int seq)
{
	char buf[200];
	sprintf(buf, "DESCRIBE rtsp://%s:%d/%s RTSP/1.0\r\n"
		"Accept: application/sdp\r\n"
		"CSeq: %d\r\n"
		"User-Agent: %s\r\n"
		"\r\n", ip, port, mediaRoute, seq, userAgent);
	return sendCmdOverTCP(buf, strlen(buf));
}

int Rtsp_client::sendCmdSetup(int seq)
{
	char buf[200];
	sprintf(buf, "SETUP rtsp://%s:%d/%s RTSP/1.0\r\n"
		"Accept: application/sdp\r\n"
		"CSeq: %d\r\n"
		"User-Agent: %s\r\n"
		"\r\n", ip, port, mediaRoute, seq, userAgent);
	return sendCmdOverTCP(buf, strlen(buf));
}

int Rtsp_client::sendCmdOverTCP(char* buff, int len)
{
	int ret = send(tcpSocket, buff, strlen(buff), 0);
	if(ret < 0)
	{
		cout << "send error:" << WSAGetLastError()<<endl;
	}
	else {
		cout <<"send-------------------\n";
		cout << buff << endl;
	}
	return ret;
}