#pragma once

#include <QTcpSocket>
#include <QThread>
#include <QTcpServer>

extern "C"
{
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libswscale/swscale.h"
}

class TcpSocket  :  public QThread
{
	Q_OBJECT

public:
	TcpSocket(QObject *parent);
	~TcpSocket();
private:
	quint32 readBufferData(quint8* buf, qint32 bufferSize);
private slots:
	
protected:
	virtual void run();
signals:
	void getConfigFrame(AVPacket *packet);
	void getFrame(AVFrame* frame);
private:
	QTcpServer *tcpServer;
	QTcpSocket *tcpSocket;

	AVCodecContext* m_codecCtx = Q_NULLPTR;
	AVPacket* m_pending = Q_NULLPTR;
	AVCodecParserContext* m_parser = Q_NULLPTR;
	AVPacket* packet = NULL;
};
