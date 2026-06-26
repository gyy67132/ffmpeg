#pragma once

#include <QObject>
#include <d3d11.h>

extern "C" {
	#include "libavcodec/avcodec.h"
	#include "libavutil/hwcontext.h"
	#include "libavutil/opt.h"
	#include "libavutil/hwcontext_d3d11va.h"
	#include "libswscale/swscale.h"
	#include "libavutil/imgutils.h"
}

#define PACKET_FLAG_CONFIG (UINT64_C(1) << 63)
#define PACKET_FLAG_KEY_FRAME (UINT64_C(1) << 62)

#define IMAGIN_W 1280
#define IMAGIN_H 720

class H264Encoder : public QObject
{
	Q_OBJECT
public:
	explicit H264Encoder(QObject* parent = nullptr);
	~H264Encoder();

	bool initialize(int width, int height, int fps, int bitrate, ID3D11Device* d3dDevice);

	void encodeFrame(ID3D11Texture2D* texture, UINT64 timestamp);

	void flush();
private:
	void sendPacket(int64_t timestamp);
	void sendConfigPacket();
	void sendWidthHeight();
signals:
	void encodedDataReady(QByteArray, uint64_t);

private:
	AVCodecContext* _ctx = nullptr;
	AVBufferRef* _hwDeviceCtx = nullptr;
	AVFrame* _frame = nullptr;
	AVPacket* _packet = nullptr;
	int _width, _height;

	ID3D11DeviceContext* _d3dContext = nullptr;
	AVFrame* _hwFrame = nullptr;
	AVFrame* _swFrame = nullptr;
	SwsContext* _swsContext = nullptr;
};

