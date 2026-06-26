#include "StreamController.h"

StreamController::StreamController(QObject* parent):QObject(parent)
{
	connect(&_timer, &QTimer::timeout, this, &StreamController::onCaptureTick);
	connect(&_encoder, &H264Encoder::encodedDataReady, this, &StreamController::onEncodedData);

	_server.listen(QHostAddress::LocalHost, 8080);
	connect(&_server, &QTcpServer::newConnection, this, &StreamController::onClientConnected);
}

void StreamController::onCaptureTick()
{
	_capturer.captureFrame();
}

void StreamController::onEncodedData(const QByteArray &data, uint64_t value)
{
	if (!_socket) return;

	if (_socket->state() != QAbstractSocket::ConnectedState) return;

	StreamPacketHeader header;
	header.flags = value;
	header.payload_size = data.size();

	if (header.payload_size > 400)
	{
		int a = 10;
		a++;
	}
	_socket->write(reinterpret_cast<const char*> (&header), HEADER_SIZE);
	if(header.payload_size > 0)
		_socket->write(data);
	_socket->flush();
}

void StreamController::stop()
{
	_isRuning = false;
	_timer.stop();
	_encoder.flush();
	_capturer.release();
}

void StreamController::onClientConnected()
{
	_socket = _server.nextPendingConnection();
	start();
}

void StreamController::start()
{
	int width = IMAGIN_W;
	int height = IMAGIN_H;
	//if (_isRuning) return;
	bool capturerOk = _capturer.initialize(width, height, [this](ID3D11Texture2D* tex, UINT64 ts) {_encoder.encodeFrame(tex, ts); });
	if (!capturerOk)
	{
		qCritical() << "Failed to initialize DXGI Capturer";
		return;
	}

	ID3D11Device* d3dDevice = _capturer.getDevice();

	bool encoderOk = _encoder.initialize(width, height, 30, 4000000, d3dDevice);
	if (!encoderOk)
	{
		qCritical() << "Failed to initialize H.264 Encoder";
		return;
	}

	_timer.start(33);
}
