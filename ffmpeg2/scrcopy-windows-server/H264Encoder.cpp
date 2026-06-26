#include "H264Encoder.h"
#include <QDebug>
#include <vector>
#include <QDateTime>
#include <QImage>

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

H264Encoder::H264Encoder(QObject *parent) : QObject(parent)
{
	av_log_set_level(AV_LOG_DEBUG);
}

H264Encoder::~H264Encoder() 
{
	av_packet_free(&_packet);
	av_frame_free(&_frame);
	avcodec_free_context(&_ctx);
	av_buffer_unref(&_hwDeviceCtx);

	if (_swsContext)
		sws_freeContext(_swsContext);
	av_frame_free(&_swFrame);
	av_frame_free(&_hwFrame);
	if (_d3dContext) _d3dContext->Release();
}

bool H264Encoder::initialize(int width, int height, int fps, int bitrate, ID3D11Device* d3dDevice)
{
	_width = width;
	_height = height;


	if (av_hwdevice_ctx_create(&_hwDeviceCtx, AV_HWDEVICE_TYPE_D3D11VA, nullptr, nullptr, 0))
	{
		qCritical() << "Failed to Create D3D11VA device context";
		return false;
	}

	AVHWDeviceContext* deviceCtx = reinterpret_cast<AVHWDeviceContext*>(_hwDeviceCtx->data);
	if (!deviceCtx)
		return false;

	AVD3D11VADeviceContext* d3d11Ctx = reinterpret_cast<AVD3D11VADeviceContext*>(deviceCtx->hwctx);
	if (!d3d11Ctx)
		return false;

	d3d11Ctx->device = d3dDevice;

	const AVCodec* codec = avcodec_find_encoder_by_name("libx264"); //avcodec_find_encoder_by_name("libx264rgb"); //avcodec_find_encoder_by_name("h264_nvenc");
	if(!codec) codec = avcodec_find_encoder_by_name("h264_qsv");
	if (!codec) codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		qCritical() << "H.264 encoder not found";
		return false;
	}

	_ctx = avcodec_alloc_context3(codec);
	_ctx->width = width;
	_ctx->height = height;
	_ctx->time_base = { 1, 90000 };
	_ctx->framerate = { fps, 1 };//帧率
	//_ctx->bit_rate = width * height * 4;//码率
	_ctx->gop_size = fps;
	_ctx->max_b_frames = 0;//禁止B帧
	/*if (strcmp(codec->name, "h264_nvenc") == 0)
		_ctx->pix_fmt = AV_PIX_FMT_D3D11;
	else if (strcmp(codec->name, "h264_qsv") == 0)
		_ctx->pix_fmt = AV_PIX_FMT_QSV;
	else */
	_ctx->pix_fmt = AV_PIX_FMT_YUV420P;// AV_PIX_FMT_BGR24;//像素格式

	if (_ctx->pix_fmt == AV_PIX_FMT_D3D11)
	{
		_ctx->hw_device_ctx = av_buffer_ref(_hwDeviceCtx);//将预先创建好的D3D11硬件设备上下文关联到当前的编码上下文中
		_ctx->hw_frames_ctx = av_hwframe_ctx_alloc(_hwDeviceCtx);
		AVHWFramesContext* fc = reinterpret_cast<AVHWFramesContext*>(_ctx->hw_frames_ctx->data);
		if (fc) {
			fc->format = AV_PIX_FMT_D3D11;
			fc->sw_format = AV_PIX_FMT_NV12;
			fc->width = width;
			fc->height = height;
			fc->initial_pool_size = 0; 
			int ret = av_hwframe_ctx_init(_ctx->hw_frames_ctx);
			if (ret != 0)
			{
				char errbuf[256];
				av_strerror(ret, errbuf, sizeof(errbuf));
				return false;
			}
		}

		av_opt_set(_ctx->priv_data, "preset", "fast", 0);
		//av_opt_set(_ctx->priv_data, "tune", "zerolatency", 0);//零延迟

	}
	av_opt_set(_ctx->priv_data, "preset", "ultrafast", 0);
	av_opt_set(_ctx->priv_data, "tune", "zerolatency", 0);
	av_opt_set(_ctx->priv_data, "crf", "16", 0);
	qDebug() << "H264 encoder =" << codec->name;
	int ret = avcodec_open2(_ctx, codec, nullptr);
	if (ret < 0) {
		char errbuf[256];
		av_strerror(ret, errbuf, sizeof(errbuf));
		return false;
	}

	_frame = av_frame_alloc();
	
	/*_hwFrame = av_frame_alloc();
	_hwFrame->format = AV_PIX_FMT_D3D11;
	_hwFrame->width = width;
	_hwFrame->height = height;
	ret = av_hwframe_get_buffer(_ctx->hw_frames_ctx, _hwFrame, 0);
	if (ret < 0) {
		char errbuf[256];
		av_strerror(ret, errbuf, sizeof(errbuf));
		return false;
	}*/
	
	

	_packet = av_packet_alloc();
	d3dDevice->GetImmediateContext(&_d3dContext);

	
	return true;
}

#include <windows.h>

HBITMAP CaptureScreen(int& w, int& h)
{
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);

	w = GetSystemMetrics(SM_CXSCREEN);
	h = GetSystemMetrics(SM_CYSCREEN);

	HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, w, h);
	HGDIOBJ old = SelectObject(hdcMem, hBmp);

	BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);

	SelectObject(hdcMem, old);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);

	return hBmp;
}

#include <vector>

std::vector<uint8_t> BitmapToBGRA(HBITMAP hBmp, int w, int h)
{
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);

	BITMAPINFOHEADER bi = {};
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = w;
	bi.biHeight = -h; // top-down
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;

	std::vector<uint8_t> pixels(w * h * 4);

	HDC hdc = GetDC(NULL);
	GetDIBits(hdc, hBmp, 0, h, pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	ReleaseDC(NULL, hdc);

	return pixels;
}
void H264Encoder::encodeFrame(ID3D11Texture2D* texture, UINT64 timestamp)
{
	if (!_ctx || !texture)
		return;

	AVFrame* inputFrame = nullptr;
	
	if (0/*_ctx->pix_fmt == AV_PIX_FMT_D3D11*/) {
		if (!_hwFrame) {
			qCritical() << "Hardware frame not allocated";
			return;
		}
		inputFrame = _hwFrame;
		
		ID3D11Texture2D* hwTexture = reinterpret_cast<ID3D11Texture2D*>(inputFrame->data);

		if (!hwTexture) return;

		if (_d3dContext) {
			_d3dContext->CopyResource(hwTexture, texture);		
		}
		else {
			qCritical() << "D3D11 Context is null, cannot copy resource";
			return;
		}

		inputFrame->pts = static_cast<int64_t>(timestamp);
	}
	else {
		/*inputFrame = _swFrame;
		inputFrame->pts = static_cast<int64_t>(timestamp);
		inputFrame->width = _width;
		inputFrame->height = _height;*/

		int w = 0, h = 0;
		HBITMAP hBmp = CaptureScreen(w, h);
		auto bgra = BitmapToBGRA(hBmp, w, h);

		/*if(!_swFrame)
		{
			_swFrame = av_frame_alloc();
			_swFrame->format = AV_PIX_FMT_BGRA;
			_swFrame->width = w;
			_swFrame->height = h;
			av_frame_get_buffer(_swFrame, 32);
		}
		
		for (int y = 0; y < h; y++) {
			memcpy(
				_swFrame->data[0] + y * _swFrame->linesize[0],
				bgra.data() + y * w * 4,
				w * 4
			);
		}*/
		
		if(!inputFrame)
		{
			inputFrame = av_frame_alloc();
			inputFrame->format = AV_PIX_FMT_YUV420P;// AV_PIX_FMT_BGR24;
			inputFrame->width = IMAGIN_W;
			inputFrame->height = IMAGIN_H;
			av_image_alloc(inputFrame->data, inputFrame->linesize, IMAGIN_W, IMAGIN_H, AV_PIX_FMT_YUV420P, 1);
			//av_frame_get_buffer(inputFrame, 32);
		}
		if (!_swsContext) {
			_swsContext = sws_getContext(w, h, AV_PIX_FMT_BGRA, IMAGIN_W, IMAGIN_H, AV_PIX_FMT_YUV420P, SWS_BICUBIC | SWS_ACCURATE_RND, nullptr, nullptr, nullptr);
		}
		/*if (1) {
			SwsContext *_swsContext2 = sws_getContext(w, h, AV_PIX_FMT_BGRA, w, h, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
			QImage image = QImage(w, h, QImage::Format_RGB888);
			uint8_t* dstData = image.bits();
			int dstLinesize = image.bytesPerLine();
			sws_scale(_swsContext2, (const uint8_t* const*)_swFrame->data, _swFrame->linesize, 0, _swFrame->height, &dstData, &dstLinesize);

			bool ret = image.save("ggy.png");

			int a = 10;
			a++;
		}*/
		const uint8_t* srcData[1] = { bgra.data() };
		int srcStride[1] = { w * 4 };
		sws_scale(_swsContext, srcData, srcStride, 0, h, inputFrame->data, inputFrame->linesize);
		
		/*QImage screenImg(bgra.data(), w, h, QImage::Format_RGBA8888);
		QImage scaled = screenImg.scaled(IMAGIN_W, IMAGIN_H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		memcpy(inputFrame->data[0], scaled.bits(), IMAGIN_W * IMAGIN_H * 4);*/


		/*D3D11_MAPPED_SUBRESOURCE mappedRes;
		HRESULT hr = _d3dContext->Map(texture, 0, D3D11_MAP_READ, 0, &mappedRes);
		if (FAILED(hr))
		{
			qCritical() << "Failed to map D3D11 texture;";
			return;
		}
		
		uint8_t* p = (uint8_t*)mappedRes.pData;
		qDebug() << "px:" << int(p[0]) << int(p[1]) << int(p[2]) << int(p[3]);

		if (!_swsContext) {
			_swsContext = sws_getContext(_width, _height, AV_PIX_FMT_BGRA, _width, _height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
		}

		const uint8_t* srcSlice = { static_cast<const uint8_t*>(mappedRes.pData) };
		int srcStride = { static_cast<int>(mappedRes.RowPitch) };

		sws_scale(_swsContext, &srcSlice, &srcStride, 0, inputFrame->height, inputFrame->data, inputFrame->linesize);

		_d3dContext->Unmap(texture, 0);*/

		
	}

	int ret = avcodec_send_frame(_ctx, inputFrame);
	if (ret < 0) {
		char errbuf[256];
		av_strerror(ret, errbuf, sizeof(errbuf));
		qCritical() << "Error sending frame to encoder"<<ret;
		return;
	}

	while (avcodec_receive_packet(_ctx, _packet) >= 0) {
		/*if (_ctx->extradata && _ctx->extradata_size > 0) {
			static int flag = 0;
			if (flag == 0) {
				flag++;
				sendConfigPacket();
			}
		}*/
		sendPacket(timestamp);
		av_packet_unref(_packet);
	}
}

void H264Encoder::sendPacket(int64_t timestamp)
{
	if (_packet->size <= 0) return;

	bool isKeyFrame = _packet->flags & AV_PKT_FLAG_KEY;
	QByteArray data(_packet->size, 0);
	memcpy(data.data(), _packet->data, _packet->size);
	uint64_t value = 0;
	if (isKeyFrame)
		value |= PACKET_FLAG_KEY_FRAME;
	//uint64_t ts = QDateTime::currentMSecsSinceEpoch();
	value |= timestamp;

	emit encodedDataReady(data, value);
}

void H264Encoder::sendConfigPacket()
{
	if (_ctx->extradata_size <= 0) return;

	uint64_t value = PACKET_FLAG_CONFIG;
	QByteArray data(reinterpret_cast<const char*>(_ctx->extradata), _ctx->extradata_size);

	emit encodedDataReady(data, value);
}

void H264Encoder::sendWidthHeight()
{
	if (_ctx->extradata_size <= 0) return;

	uint64_t height = _ctx->height;
	uint64_t value = _ctx->width;
	value |= (height << 32);
	QByteArray data;

	emit encodedDataReady(data, value);
}

void H264Encoder::flush()
{
	avcodec_send_frame(_ctx, nullptr);

	while (avcodec_receive_packet(_ctx, _packet) >= 0) {
		sendPacket(0);
		av_packet_unref(_packet);
	}
}
