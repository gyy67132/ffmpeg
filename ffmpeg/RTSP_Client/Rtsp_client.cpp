#include "Rtsp_client.h"

using namespace std;

#include "librtp/rtp-payload.h"
#include <assert.h>

static void* rtp_packet_alloc(void *, int bytes)
{
	static uint8_t buffer[2 * 1024 * 1024] = {0,0,0,1};
	assert(bytes <= sizeof(buffer) - 4);
	return buffer + 4;
}

static void rtp_packet_free(void *, void *)
{
}

static int rtp_packet_decode_packet(void* param, const void* packet, int bytes, uint32_t timestamp, int flags)
{
	static uint8_t buffer[8 * 1024 * 1024];

	assert(bytes + 4 < sizeof(buffer));
	assert(0 == flags);

	static const uint8_t start_code[4] = { 0x00,0x00,0x00,0x01 };
	struct RtpContext* ctx = (struct RtpContext*)param;

	size_t buffer_size = 0;

	if (0 == strcmp("H264", ctx->encoding) || 0 == strcmp("H265", ctx->encoding))
	{
		memcpy(buffer, start_code, sizeof(start_code));
		buffer_size += sizeof(start_code);
	}
	memcpy(buffer + buffer_size, packet, bytes);
	buffer_size += bytes;

	uint8_t naluType = buffer[4];

	if (0x65 == naluType)
	{
		//20 20 20 01 67 64 20 28 ac d9 40 b4 3d bf f0 02 20 01 b1 20 20 03 20 01 20 20 03 20 32 0f 18 31 96
		uint8_t buff[33] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x28, 0xac, 0xd9, 0x40, 0xb4, 0x3d, 0xbf, 0xf0, 0x02, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x03,
							0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x32, 0x0f, 0x18, 0x31, 0x96 };
		fwrite(buff, 1, sizeof(buff), ctx->out_f);
		//00 00 00 01 68 eb ec b2 2c
		uint8_t buff2[9] = { 0x00, 0x00, 0x00, 0x01, 0x68, 0xeb, 0xec, 0xb2, 0x2c };
		fwrite(buff2, 1, sizeof(buff2), ctx->out_f);
	}

	fwrite(buffer, 1, sizeof(buffer), ctx->out_f);

	return 0;
}

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

	m_rtpCtx = new RtpContext;
	m_rtpCtx->playload = 96;
	m_rtpCtx->encoding = "H264";
	m_rtpCtx->out_f = fopen("out.h264", "wb");

	rtp_packet_setsize(1456);

	struct rtp_payload_t handler;
	handler.alloc = rtp_packet_alloc;
	handler.free = rtp_packet_free;
	handler.packet = rtp_packet_decode_packet;
	m_rtpCtx->decoder = rtp_payload_decode_create(m_rtpCtx->playload, m_rtpCtx->encoding, &handler, m_rtpCtx);
}
Rtsp_client::~Rtsp_client()
{
	closesocket(tcpSocket);
	WSACleanup();

	rtp_payload_decode_destroy(m_rtpCtx->decoder);
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
	bool isSendPlay = false;
	char recvBufCopy[buffSize] = {0};

	while (1)
	{
		int ret = recv(tcpSocket, recvBuf, buffSize, 0);
		if (ret <= 0)
		{
			std::cerr << "recv error\n";
			return;
		}
		/*if(ret == buffSize)
			recvBuf[ret - 1] = '\0';
		else*/
			recvBuf[ret] = '\0';
		printf("recv-------------------\n%s",recvBuf);

		responseStateCode = 0;
		recvSeq = 0;
		memcpy(recvBufCopy, recvBuf, ret+1);

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
			else if (strstr(line, "Session:"))
			{
				if (sscanf(line, "Session: %s", session) != 1) {
					std::cerr << "parse session error\n";
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
				
				sdp.parse(recvBufCopy, ret);
				SdpTrack* track = sdp.popTrack();

				if (track)
				{
					//第一次发送setup
					sendCmdSetup(++sendSeq, track);
				}
			}
			else if (sendSeq == recvSeq && !isSendPlay)
			{
				SdpTrack* track = sdp.popTrack();
				if (track)
				{
					//第二次发送setup
					sendCmdSetup(++sendSeq, track);
				}
				else {
					if (sendCmdPlay(++sendSeq) < 0)
					{
						std::cerr << "sendCmdPlay error "<< endl;
						goto FINISH;
					}
					isSendPlay = true;
				}
			}
			else if (sendSeq == recvSeq && isSendPlay)
			{
				std::cout << "play......."<< endl;

				parseData();
				goto FINISH;
			}
			else {
				std::cerr << "recvSeq error " << recvSeq<< endl;
				goto FINISH;
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

int Rtsp_client::sendCmdSetup(int seq, SdpTrack* track)
{
	int interleaved = track->control_id * 2;
	
	char buf[200];
	sprintf(buf, "SETUP rtsp://%s:%d/%s/%s RTSP/1.0\r\n"
		"Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n"
		"CSeq: %d\r\n"
		"User-Agent: %s\r\n"
		"Session: %s\r\n"
		"\r\n", ip, port, mediaRoute,track->control, interleaved, interleaved + 1, seq, userAgent, session);
	return sendCmdOverTCP(buf, strlen(buf));
}

int Rtsp_client::sendCmdPlay(int seq)
{
	char buf[200];
	sprintf(buf, "PLAY rtsp://%s:%d/%s RTSP/1.0\r\n"
		"Range: npt=0.000-\r\n"
		"CSeq: %d\r\n"
		"User-Agent: %s\r\n"
		"Session: %s\r\n"
		"\r\n", ip, port, mediaRoute, seq, userAgent, session);
	return sendCmdOverTCP(buf, strlen(buf));
}

int Rtsp_client::sendCmdOverTCP(char* buff, int len)
{
	int ret = send(tcpSocket, buff, strlen(buff), 0);
	if(ret < 0)
	{
		printf( "send error: %d\n", WSAGetLastError());
	}
	else {
		printf("send-------------------\n%s", buff);
	}
	return ret;
}

void Rtsp_client::parseData()
{
	const int buffSize = 1024;
	char *recvBuff = (char*)malloc(buffSize);
	while (1)
	{
		int ret = recv(tcpSocket, recvBuff, buffSize, 0);
		if (ret <= 0)
		{
			printf("recv error %d\n", WSAGetLastError());
			return;
		}
		
		printf("\nparseData recv-------\n%s", recvBuff);
		char* p = recvBuff;
		int size = ret;
		while (size != 0)
		{
			if ('$' == *p)
			{
				if (size < 4)
				{
					int need = 4 - size;
					while (need) {
						ret = recv(tcpSocket, p + size, need, 0);
						if (ret <= 0)
						{
							printf("recv(2) error %d\n", WSAGetLastError());
							return;
						}
						size += ret;
						need -= ret;
					}
				}
				++p;
				int8_t channel = *p;
				++p;
				int16_t len =ntohs(*(int16_t*)p);
				p += 2;
				size -= 4;

				if (size >= len)
				{
					p += len;
					size -= len;
					parsePacket(channel, p, len);
				}
				else {
					int need = len - size;
					while (need)
					{
						ret = recv(tcpSocket, p + size, need, 0);
						if (ret <= 0)
						{
							printf("recv(3) error %d\n", WSAGetLastError());
							return;
						}
						size += ret;
						need -= ret;
					}

					parsePacket(channel, p, len);
					p += len;
					size = 0;
				}
				
			}
			else {
				p++;
			}
		}
	}

	free(recvBuff);
}

bool Rtsp_client::parsePacket(char channel, char* packet, int size)
{
	if (0 == channel)
	{
		m_rtpCtx->size = size;
		memcpy(m_rtpCtx->packet, packet, size);

		rtp_payload_decode_input(m_rtpCtx->decoder, m_rtpCtx->packet, m_rtpCtx->size);
	}
	else {

	}
	return true;
}