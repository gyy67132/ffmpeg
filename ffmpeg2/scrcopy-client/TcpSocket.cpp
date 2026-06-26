#include "TcpSocket.h"

#include <QHostAddress>
#include <QDataStream>


#include "Thread.h"


TcpSocket::TcpSocket()
{
	connect(this, &QTcpSocket::connected, this, &TcpSocket::connectedServer);
	this->connectToHost(QHostAddress::LocalHost, 8080);

	
}

void TcpSocket::connectedServer()
{
	Thread* thread = new Thread;
	thread->setSocket(this);
	this->moveToThread(thread);
	thread->moveToThread(thread);
	thread->start();	
	connect(thread, &Thread::signal_image, this, &TcpSocket::signal_updateImage);
	connect(thread, &Thread::signal_frame, this, &TcpSocket::signal_updateFrame);

	QMetaObject::invokeMethod(thread, "doDecode", Qt::QueuedConnection);
}

void TcpSocket::updateImage(QImage image)
{
	//label->setPixmap(QPixmap::fromImage(image));
		//label->update();
	//w->updateImage(image);
}

bool TcpSocket::readHeader(StreamPacketHeader * streamPacketHeader)
{
	while (bytesAvailable() < 12)
	{
		if (!waitForReadyRead(-1))
			return false;
	}

	QByteArray byte = read(12);

	QDataStream in(byte);
	in.setByteOrder(QDataStream::LittleEndian);

	in >> streamPacketHeader->flags >> streamPacketHeader->payload_size;

	return byte.size() == sizeof(StreamPacketHeader)? true : false;
}

bool TcpSocket::readBody(uint8_t* buf, qint32 bufSize)
{
	if (!buf || bufSize < 0)
		return false;
	
	while (bytesAvailable() < bufSize)
	{
		if (!waitForReadyRead(-1))
			return false;
	}
	int ret = read((char*)buf, bufSize);
	return ret == bufSize ? true : false;
}

bool TcpSocket::readWidthHeight(int &width, int &height)
{
	while (bytesAvailable() < 12)
	{
		if (!waitForReadyRead(-1))
			return false;
	}

	QByteArray byte = read(12);

	QDataStream in(byte);
	in.setByteOrder(QDataStream::LittleEndian);

	in >> width >> height;

	return byte.size() == sizeof(StreamPacketHeader) ? true : false;
}