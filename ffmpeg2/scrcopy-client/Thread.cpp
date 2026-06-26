#include "Thread.h"

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

#include <QImage>

Thread::Thread()
{

}

Thread::~Thread()
{

}

void Thread::setSocket(TcpSocket* socket)
{
	socket->moveToThread(this);
	_socket = socket;
}

void Thread::run()
{
	exec();
}
void Thread::doDecode()

{
	const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	AVPacket* packet = nullptr;
	AVFrame* frame = av_frame_alloc();
	
	AVFrame* rgbFrame = av_frame_alloc();
	rgbFrame->format = AV_PIX_FMT_YUV420P;// AV_PIX_FMT_BGRA;
	rgbFrame->width = IMAGIN_W;
	rgbFrame->height = IMAGIN_H;
	av_image_alloc(rgbFrame->data, rgbFrame->linesize, IMAGIN_W, IMAGIN_H, AV_PIX_FMT_YUV420P, 1);
	/*uint8_t* rgbaBuf = new uint8_t[IMAGIN_W * IMAGIN_H * 4];
	rgbFrame->data[0] = rgbaBuf;
	rgbFrame->linesize[0] = IMAGIN_W * 4;*/

	packet = av_packet_alloc();
	struct SwsContext* sws_ctx = nullptr;
	QImage image;// = QImage(IMAGIN_W, IMAGIN_H, QImage::Format_RGB32);
	uint8_t* dstData = nullptr;
	int dstLinesize;

	if (!packet)
	{
		goto runQuit;
	}
	//parser = av_parser_init(AV_CODEC_ID_H264);
	//parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
	codecCtx = avcodec_alloc_context3(codec);

	if (avcodec_open2(codecCtx, codec, NULL) < 0) {
		return;
	}

	StreamPacketHeader header;
	while (1)
	{
		bool ok = _socket->readHeader(&header);
		if (!ok) {
			break;
		}
		if (int ret = av_new_packet(packet, header.payload_size)) {
			char errbuf[256];
			av_strerror(ret, errbuf, sizeof(errbuf));
			break;
		}
		ok = _socket->readBody(packet->data, header.payload_size);
		if (header.payload_size > 300)
		{
			int a = 10;
			a++;
		}
		if (!ok)
		{
			av_packet_unref(packet);
			continue;
		}

		bool isConfig = header.flags & PACKET_FLAG_CONFIG ? true : false;
		if (isConfig)//配置包
		{
			packet->pts = AV_NOPTS_VALUE;
		}
		else {
			packet->pts = header.flags & PACKET_PTS_MASK;
		}

		if (header.flags & PACKET_FLAG_KEY_FRAME) {
			packet->flags |= AV_PKT_FLAG_KEY;
		}
		packet->dts = packet->pts;


		//if (isConfig || m_pending)//配置包 pps/sps包需要后面数据包一起才能解码
		//{
		//	int offset;
		//	if (!m_pending) {
		//		offset = 0;
		//		m_pending = av_packet_alloc();
		//		if (av_new_packet(m_pending, packet->size)) {
		//			av_packet_free(&m_pending);
		//			continue;
		//		}
		//	}
		//	else {
		//		offset = m_pending->size;
		//		if (av_grow_packet(m_pending, packet->size))
		//		{
		//			av_packet_free(&m_pending);
		//			continue;
		//		}
		//	}
		//	memcpy(m_pending->data + offset, packet->data, packet->size);

		//	if (!isConfig) {
		//		m_pending->pts = packet->pts;
		//		m_pending->dts = packet->dts;
		//		m_pending->flags = packet->flags;
		//		packet = m_pending;
		//	}
		//}

		//if (isConfig) {
			//
		//}
		//else {
			
			/*uint8_t* outData = nullptr;
			int outLen = 0;
			int r = av_parser_parse2(parser, codecCtx, &outData, &outLen, packet->data, packet->size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, -1);
			if (r != packet->size || outLen != packet->size) {
				break;
			}

			if (parser->key_frame == 1)
				packet->flags |= AV_PKT_FLAG_KEY;
			packet->dts = parser->pts;*/
			int ret = avcodec_send_packet(codecCtx, packet);
			if (ret != 0) {
				break;
			}
			while (1) {
				ret = avcodec_receive_frame(codecCtx, frame);
				if (ret != 0) {
					break;
				}

				if (!sws_ctx)
				{
					sws_ctx = sws_getContext(codecCtx->width, codecCtx->height, (AVPixelFormat)codecCtx->pix_fmt, IMAGIN_W, IMAGIN_H, AV_PIX_FMT_YUV420P /*AV_PIX_FMT_BGRA*/, SWS_BICUBIC | SWS_ACCURATE_RND, NULL, NULL, NULL);
				}
				dstData = image.bits();
				dstLinesize = image.bytesPerLine();

				ret = sws_scale(sws_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, codecCtx->height, rgbFrame->data, rgbFrame->linesize);

				//ret = sws_scale(sws_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, &dstData, &dstLinesize);
				Qt::HANDLE id = QThread::currentThreadId();
				QImage img(rgbFrame->data[0], rgbFrame->width, rgbFrame->height, rgbFrame->linesize[0], QImage::Format_RGBA8888);
				//qDebug() << frame->linesize[0]<< frame->linesize[1]<< frame->linesize[2] << frame->width <<frame->height<< image.width() << image.height() << rgbFrame->linesize[0] << image.bytesPerLine()<< rgbFrame->width<<frame->height;
				emit signal_image(img.copy());
				emit signal_frame(rgbFrame);
				
			}
			/*if (m_pending)
			{
				av_packet_free(&m_pending);
			}*/
		//}


	}

runQuit:
	av_frame_free(&frame);
}