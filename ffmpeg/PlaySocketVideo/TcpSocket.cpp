#include "TcpSocket.h"

#include <QMessageLogger>
#include <QHostAddress>

#define HEADER_SIZE 12

#define SC_PACKET_FLAG_CONFIG    (UINT64_C(1) << 63)
#define SC_PACKET_FLAG_KEY_FRAME (UINT64_C(1) << 62)

#define SC_PACKET_PTS_MASK (SC_PACKET_FLAG_KEY_FRAME - 1)

static quint32 bufferRead32be(quint8* buf)
{
	return static_cast<quint32>((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}

static quint64 bufferRead64be(quint8* buf)
{
	quint32 msb = bufferRead32be(buf);
	quint32 lsb = bufferRead32be(&buf[4]);
	return (static_cast<quint64>(msb) << 32) | lsb;
}

TcpSocket::TcpSocket(QObject *parent)
	:QThread()
{
	bool connected = false;
	tcpServer = new QTcpServer;
	tcpServer->listen(QHostAddress::LocalHost, 8000);
	tcpServer->setMaxPendingConnections(10);
	QObject::connect(tcpServer, &QTcpServer::newConnection, this, [=]() {
		tcpSocket = tcpServer->nextPendingConnection();
		//tcpSocket->moveToThread(this);
		tcpSocket->waitForReadyRead();
		tcpSocket->setParent(nullptr);
		tcpSocket->moveToThread(this);
		start();
		connect(tcpSocket, &QTcpSocket::connected, this, [=]() {
			int a = 10;
			qDebug() << "connected";
			});
		//connect(tcpSocket, &QTcpSocket::readyRead, this, [=]() {
		//	int a = 10;
		//	//QString str = tcpSocket->readAll();
		//	qDebug() <<"readyRead:";
		//
		//	});
		});

}

TcpSocket::~TcpSocket()
{

}

void TcpSocket::run()
{
	const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		qDebug() << "000";
		return;
	}
	m_codecCtx = avcodec_alloc_context3(codec);
	if (!m_codecCtx)
	{
		qDebug() << "0000";
		return;
	}
	m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
	m_codecCtx->width = 1080;
	m_codecCtx->height = 608;
	m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (avcodec_open2(m_codecCtx, codec, NULL) < 0)
	{
		qDebug() << "00000";
		return;
	}

	packet = av_packet_alloc();
	if (!packet)
	{
		qDebug() << "001";
		return;
	}

	m_parser = av_parser_init(AV_CODEC_ID_H264);
	if (!m_parser)
	{
		qDebug() << "002";
		return;
	}

	m_parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;

	AVFrame* m_decodingFrame = av_frame_alloc();
	AVFrame* frame = av_frame_alloc();
	frame->width = m_codecCtx->width;
	frame->height = m_codecCtx->height;
	frame->format = AV_PIX_FMT_RGB24;
	av_frame_get_buffer(frame, 1);

	struct SwsContext *sws_ctx = sws_getContext(m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt, m_codecCtx->width, m_codecCtx->height,
		AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

	while (1)
	{
		qDebug() << "read......";
		quint8 header[HEADER_SIZE];
		quint32 r = readBufferData(header, HEADER_SIZE);
		if (r < HEADER_SIZE)
		{
			qDebug() << "11";
			return;
		}
		quint64 ptsFlags = bufferRead64be(header);
		quint32 len = bufferRead32be(&header[8]);

		if (len<=0) {
			qDebug() << "22";
			break;
		}
		if (av_new_packet(packet, static_cast<int>(len))) {
			qDebug() << "33"<< len;
			break;
		}
		if (packet->size != len)
		{
			int a = 10;
			a++;
		}
		//qCritical("packet->size:%d,%d",packet->size, len);

		r = readBufferData(packet->data, static_cast<qint32>(len));
		if (r < len) {
			qDebug() << "44";
			break;
		}
		if (r != len)
		{
			int a = 10;
			a++;
		}

		if (packet->size != len)
		{
			quint32 len1 = len;
			int a = 10;
			a++;
			continue;
		}else {
			int a = 10;
			a++;
		}

		if (ptsFlags & SC_PACKET_FLAG_CONFIG)
			packet->pts = AV_NOPTS_VALUE;
		else
			packet->pts = ptsFlags & SC_PACKET_PTS_MASK;

		if (ptsFlags & SC_PACKET_FLAG_KEY_FRAME)
			packet->flags |= AV_PKT_FLAG_KEY;

		packet->dts = packet->pts;


		bool isConfig = packet->pts == AV_NOPTS_VALUE;

		static int ff = 0;
		if (m_pending || isConfig)
		{
			qDebug() << "ggy--1-------------------------------------------------------" << ff++;
			qint32 offset;
			if (m_pending) {
				offset = m_pending->size;
				if (av_grow_packet(m_pending, packet->size))
				{
					qDebug() << "55";
					break;
				}
			}
			else {
				offset = 0;
				m_pending = av_packet_alloc();
				if (av_new_packet(m_pending, packet->size))
				{
					av_packet_free(&m_pending);
					qDebug() << "66";
					break;
				}
			}
			if (packet->size != len)
			{
				quint32 len1 = len;
				int a = 10;
				a++;
				continue;
			}
			memcpy(m_pending->data + offset, packet->data, packet->size);

			if (!isConfig)
			{
				m_pending->pts = packet->pts;
				m_pending->dts = packet->dts;
				m_pending->flags = packet->flags;
				packet = m_pending;
			}
		}

		if (isConfig)
		{
			qDebug() << "ggy--2-------------------------------------------------------" << ff++;
			emit getConfigFrame(packet);
		}
		else {
			quint8* inData = packet->data;
			int inLen = packet->size;
			quint8* outData = Q_NULLPTR;
			int outLen = 0;
			int r = av_parser_parse2(m_parser, m_codecCtx, &outData, &outLen, inData, inLen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, -1);

			if (r != inLen || outLen != inLen)
			{
				qDebug() << "77";
				break;
			}

			if (m_parser->key_frame == 1)
				packet->flags |= AV_PKT_FLAG_KEY;

			packet->dts = packet->pts;

			if (avcodec_send_packet(m_codecCtx, packet) < 0)
			{
				qDebug() << "88";
				break;

			}
			if (avcodec_receive_frame(m_codecCtx, m_decodingFrame) != 0)
			{
				qDebug() << "99";
				break;
			}

			sws_scale(sws_ctx, (const uint8_t* const*)m_decodingFrame->data, m_decodingFrame->linesize, 0, m_codecCtx->height, frame->data, frame->linesize);

			emit getFrame(frame);
			qDebug() << "ggy--3-------------------------------------------------------" << ff++<<frame->linesize[0] << m_codecCtx->height;

			if (m_pending)
				av_packet_free(&m_pending);
		}

		av_packet_unref(packet);
	}

	if (m_pending)
		av_packet_free(&m_pending);
	av_packet_free(&packet);

	av_parser_close(m_parser);
FINISH:
	if(m_codecCtx)
		avcodec_free_context(&m_codecCtx);
}

quint32 TcpSocket::readBufferData(quint8* buf, qint32 bufferSize)
{
	
	while (tcpSocket->bytesAvailable() < bufferSize)
	{
		if (!tcpSocket->waitForReadyRead(-1)) {
			return 0;
		}
	}
	quint32 ret = tcpSocket->read((char*)buf, bufferSize);
	qDebug() << "readBufferData:" << bufferSize << ret;
	return ret;
}
