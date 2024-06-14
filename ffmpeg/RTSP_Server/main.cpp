#include <winsock2.h>
#include <stdio.h>
#include <time.h>

#include "RTP.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8554
#define SERVER_RTP_PORT 55532
#define SERVER_RTCP_PORT 55533

#define FRAME_SIZE 50000

int startWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		printf("WSAStartup failed with error: %d\n", err);
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("Could not find a useable version of Winsock.dll\n");
		return -1;
	}
	return 0;
}

SOCKET createTCPSocket()
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("tcp socket error\n");
		WSACleanup();
		return SOCKET_ERROR;
	}
	return s;
}

SOCKET createUDPSocket()
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("udp socket error\n");
		WSACleanup();
		return SOCKET_ERROR;
	}
	int on = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
	return s;
}

int bindSocketAddr(int sockfd, const char *ip, int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);
	if (SOCKET_ERROR == bind(sockfd, (const sockaddr*)&addr, sizeof(addr)))
	{
		printf("bind error\n");
		closesocket(sockfd);
		WSACleanup();
		return -1;
	}
	return 0;
}


void handleCMD_OPTIONS(char* result, int cseq)
{
	if (!result)
		return;

	sprintf(result, "RTSP/1.0 200 OK\r\n"
				"CSeq: %d\r\n"
				"Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
				"\r\n", cseq);
}

void handleCMD_DESCRIBE(char* result, int cseq, char *url)
{
	if (!result || !url)
		return;

	char sdp[500];
	char localIp[100];

	sscanf(url, "rtsp://%[^:]", localIp);

	sprintf(sdp, "v=0\r\n"
		"o=- 9%ld 1 IN IP4 %s\r\n"
		"t=0 0\r\n"
		"a=control:*\r\n"
		"m=video 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 H264/90000\r\n"
		"a=control:track0\r\n", time(NULL), localIp);

	sprintf(result, "RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Content-Base: %s\r\n"
		"Content-type: application/sdp\r\n"
		"Content-length: %zu\r\n\r\n"
		"%s", cseq,url,strlen(sdp),sdp);
}


void handleCMD_SETUP(char* result, int cseq, int clientRtpProt)
{
	sprintf(result, "RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
		"Session: 66334873\r\n"
		"\r\n", cseq, clientRtpProt, clientRtpProt+1, SERVER_RTP_PORT, SERVER_RTCP_PORT);
}

void handleCMD_PLAY(char* result, int cseq)
{
	sprintf(result, "RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Range: npt=0.000-\r\n"
		"Session: 66334873; timeout=10\r\n\r\n", cseq);
}

int startCode3(char* frame)
{
	if (frame[0] == 0x00 && frame[1] == 0x00 && frame[2] == 0x01)
		return 1;
	return 0;
}

int startCode4(char* frame)
{
	if (frame[0] == 0x00 && frame[1] == 0x00 && frame[2] == 0x00 && frame[3] == 0x01)
		return 1;
	return 0;
}

char* findNextStartCode(char* frame, int len)
{
	/*if (len < 3)
		return NULL;*/
	while (1)
	{
		if (startCode3(frame) || startCode4(frame))
			break;
		frame++;
	}
	return frame;
}

int getFrameFormH264File(FILE* fp, char *frame, uint32_t size)
{
	if (fp < 0)
		return -1;

	int rSize = fread(frame, 1, size, fp);
	if (rSize <= 0)
		return -1;

	if (!startCode3(frame) && !startCode4(frame))
		return -1;

	int frameSize;
	char* nextStartCode = findNextStartCode(frame + 3, rSize - 3);
	if (!nextStartCode)
		return -1;

	frameSize = nextStartCode - frame;
	fseek(fp, frameSize - size, SEEK_CUR);

	return frameSize;
}

int sendRtpH264Frame(SOCKET serverRtpSocket, char *clientIp, uint16_t clientPort,
	struct RtpPacket *rtpPacket,char *frame, int frameSize)
{
	int ret;
	int sendBytes = 0;
	
	uint8_t naluType = frame[0];
	
	if (frameSize <= RTP_MAX_PKT_SIZE)
	{
		memcpy(rtpPacket->payload, frame, frameSize);
		ret = sendRtpPacketOverUDP(serverRtpSocket, clientIp, clientPort, rtpPacket, frameSize + RTP_HEADER_SIZE);
		if (ret < 0)
			return -1;

		sendBytes += ret;
		rtpPacket->rtpHeader.seq++;

		if ((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8)
			goto out;
	}
	else {
		int ptkNum = frameSize / RTP_MAX_PKT_SIZE;
		int remainrPtkSize = frameSize % RTP_MAX_PKT_SIZE;
		int pos = 1;

		for (int i = 0; i < ptkNum; i++)
		{
			rtpPacket->payload[0] = (naluType & 0x60) | 28;
			rtpPacket->payload[1] = naluType & 0x1F;

			if (i == 0)
				rtpPacket->payload[1] |= 0x80;
			else if(i == ptkNum - 1 && remainrPtkSize == 0)
				rtpPacket->payload[1] |= 0x40;

			memcpy(rtpPacket->payload + 2, frame + pos, RTP_MAX_PKT_SIZE);
			ret = sendRtpPacketOverUDP(serverRtpSocket, clientIp, clientPort, rtpPacket, RTP_MAX_PKT_SIZE + 2);
			if (ret < 0)
				return -1;

			sendBytes += ret;
			rtpPacket->rtpHeader.seq++;
			pos += RTP_MAX_PKT_SIZE;
		}

		if (remainrPtkSize > 0)
		{
			rtpPacket->payload[0] = (naluType & 0x60) | 28;
			rtpPacket->payload[1] = naluType & 0x1F;
			rtpPacket->payload[1] |= 0x40;

			memcpy(rtpPacket->payload + 2, frame + pos, remainrPtkSize);
			ret = sendRtpPacketOverUDP(serverRtpSocket, clientIp, clientPort, rtpPacket, remainrPtkSize + 2);
			if (ret < 0)
				return -1;

			sendBytes += ret;
			rtpPacket->rtpHeader.seq++;
		}
	}

	rtpPacket->rtpHeader.timestamp += 90000 / 25;

out:
	return sendBytes;
}

void doClient(SOCKET clientSocket, char* clientIp, uint16_t clientPort)
{
	const int buffsize = 1024;
	char recvBuff[buffsize];
	char sendBuff[buffsize];

	char method[128];
	char url[128];
	char version[128];

	int cseq;
	int clientRtpPort, clientRtcpPort;
	int serverRtpSocket = -1, serverRtcpSocket = -1;
	
	while (1)
	{
		int recvLen;
		recvLen = recv(clientSocket, recvBuff, buffsize, 0);
		if (recvLen <= 0 || recvLen >= buffsize)
		{
			printf("recv error %d", recvLen);
			break;
		}
		recvBuff[recvLen] = '\0';
		printf("recv-----------:\n%s\n", recvBuff);

		const char* sep = "\n";
		char *line = strtok(recvBuff, sep);
		while (line) {
			
			if (strstr(line, "OPTIONS") || 
				strstr(line, "DESCRIBE") ||
				strstr(line, "SETUP") ||
				strstr(line, "PLAY"))
			{
				if (sscanf(line, "%s %s %s", method, url, version) != 3)
				{
				}
			}

			if (strstr(line, "CSeq"))
			{
				if (sscanf(line, "CSeq: %d\r\n", &cseq) != 1)
				{
				}
			}

			if (strstr(line, "Transport:"))
			{
				if (sscanf(line, "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d", &clientRtpPort, &clientRtcpPort) != 2)
				{
				}
			}

			line = strtok(NULL, sep);
		}

		if (!strcmp(method, "OPTIONS"))
		{
			handleCMD_OPTIONS(sendBuff, cseq);
		}else if (!strcmp(method, "DESCRIBE"))
		{
			handleCMD_DESCRIBE(sendBuff, cseq, url);
		}else if (!strcmp(method, "SETUP"))
		{
			handleCMD_SETUP(sendBuff, cseq, clientRtpPort);
			serverRtcpSocket = createUDPSocket();
			serverRtpSocket = createUDPSocket();

			if (serverRtpSocket == INVALID_SOCKET || serverRtcpSocket == INVALID_SOCKET)
			{
				printf("serverRtcpSocket or serverRtpSocket error\n");
				break;
			}

			if (bindSocketAddr(serverRtpSocket, "0.0.0.0", SERVER_RTP_PORT) < 0 ||
				bindSocketAddr(serverRtcpSocket, "0.0.0.0", SERVER_RTCP_PORT) < 0)
			{
				printf("serverRtcpSocket or serverRtpSocket bind error\n");
				break;
			}

		}
		else if (!strcmp(method, "PLAY"))
		{
			handleCMD_PLAY(sendBuff, cseq);
		}
		else {
			printf("未定义method = %s \n", method);
			break;
		}

		if (strlen(sendBuff) > 0) {
			send(clientSocket, sendBuff, strlen(sendBuff), 0);
			printf("send-----------:\n%s\n", sendBuff);
		}
		
		if (!strcmp(method, "PLAY"))
		{
			FILE* fp = fopen("./1.h264", "rb");
			if (!fp)
			{
				printf("读取1.h264文件失败\n");
				break;
			}

			struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(FRAME_SIZE);
			rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VERSION, RTP_PLAYLOAD_TYPE_H264, 0, 0, 0, 0x88923423);

			char *frame = (char*)malloc(FRAME_SIZE);

			int frameSize;

			while (1)
			{
				frameSize = getFrameFormH264File(fp, frame, FRAME_SIZE);
				if (frameSize < 0)
				{
					printf("读取文件结束\n");
					break;
				}

				int startCode = 0;
				if (startCode3(frame))
					startCode = 3;
				else if (startCode4(frame))
					startCode = 4;
				frameSize -= startCode;
				if (sendRtpH264Frame(serverRtpSocket, clientIp, clientPort,
					rtpPacket, frame + startCode, frameSize) < 0)
					break;
				
				Sleep(40);
			}

			free(frame);
			free(rtpPacket);
			fclose(fp);
			
		}

		memset(sendBuff, 0, buffsize);
		memset(method, 0, sizeof(method)/sizeof(char));
		memset(url, 0, sizeof(url) / sizeof(char));
		cseq = 0;
	}
	closesocket(clientSocket);
}

int main()
{
	if (startWinsock())
		return -1;
	SOCKET tcpsocket = createTCPSocket();
	if (tcpsocket == SOCKET_ERROR)
		return -1;
	int on = 1;
	setsockopt(tcpsocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	addr.sin_port = htons(SERVER_PORT);
	if (SOCKET_ERROR == bind(tcpsocket, (const sockaddr*)&addr, sizeof(addr)))
	{
		printf("bind error\n");
		closesocket(tcpsocket);
		WSACleanup();
		return -1;
	}
	if (SOCKET_ERROR == listen(tcpsocket, 10))
	{
		printf("listen error\n");
		closesocket(tcpsocket);
		WSACleanup();
		return -1;
	}

	printf("%s rtsp://127.0.0.1:%d\n", __FILE__, SERVER_PORT);

	while (1)
	{
		sockaddr_in addrclient;
		int size = sizeof(addrclient);
		SOCKET socketClient = accept(tcpsocket, (sockaddr*)&addrclient, &size);
		if (INVALID_SOCKET == socketClient)
		{
			printf("与客户端数据传送的套接字建立失败，错误代码为：%d\n", WSAGetLastError());
			break;
		}
		char* clientIp = inet_ntoa(addrclient.sin_addr);
		uint16_t clientPort = ntohs(addrclient.sin_port);
		doClient(socketClient, clientIp, clientPort);
	}

	closesocket(tcpsocket);
	WSACleanup();

	return 0;
}
