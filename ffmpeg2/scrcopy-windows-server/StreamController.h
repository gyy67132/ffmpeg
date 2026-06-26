#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include "DxgiCapturer.h"
#include "H264Encoder.h"
#include <cstdint>

#define PACKET_FLAG_CONFIG (UINT64_C(1) << 63)
#define PACKET_FLAG_KEY_FRAME (UINT64_C(1) << 62)

#pragma pack(push, 1)
struct StreamPacketHeader {
	//uint32_t flags;
	//uint32_t timestamp;
	uint64_t flags;
	uint32_t payload_size;
};
#pragma pack(pop)

constexpr size_t HEADER_SIZE = sizeof(StreamPacketHeader);
constexpr uint32_t FLAG_KEY_FRAME = 0x01;
constexpr uint32_t FLAG_END_FRAME = 0x02;


class StreamController : public QObject 
{
	Q_OBJECT
public:
	explicit StreamController(QObject* parent = nullptr);
	void stop();
	void start();

private slots:
	void onCaptureTick();
	void onEncodedData(const QByteArray& data, uint64_t value);
	void onClientConnected();
private:
	DxgiCapturer _capturer;
	H264Encoder _encoder;
	QTcpSocket *_socket = nullptr;
	QTimer _timer;
	bool _isRuning = false;
	QTcpServer _server;
};

