#include "FFmpeg.h"


FFmpeg::FFmpeg(QObject *parent)
	: QObject(parent)
{
	
	if (0 != avformat_open_input(&avf, "./11.mp4", NULL, NULL))
		return;
	if (avformat_find_stream_info(avf, NULL) < 0)
		return;
	
	streamIndex = av_find_best_stream(avf, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, NULL);
	if (streamIndex == AVERROR_STREAM_NOT_FOUND)
		return;
	
	AVCodecParameters *avcodecP = avf->streams[streamIndex]->codecpar;
	if (!avcodecP)
		return;

	const AVCodec *pCodec = avcodec_find_decoder(avcodecP->codec_id);

	avCodecCtx = avcodec_alloc_context3(pCodec);

	avcodec_parameters_to_context(avCodecCtx, avcodecP);

	avcodec_open2(avCodecCtx, pCodec, NULL);

	frame = av_frame_alloc();
	frame2 = av_frame_alloc();
	frame2->width = avCodecCtx->width;
	frame2->height = avCodecCtx->height;
	frame2->format = AV_PIX_FMT_RGB24;
	av_frame_get_buffer(frame2, 1);

	sws_ctx = sws_getContext(avCodecCtx->width, avCodecCtx->height, avCodecCtx->pix_fmt, avCodecCtx->width, avCodecCtx->height, 
		AV_PIX_FMT_RGB24,SWS_BILINEAR, NULL, NULL, NULL);
	avpacket = av_packet_alloc();

	startTimer(15);
}

FFmpeg::~FFmpeg()
{
	avformat_close_input(&avf);
	avcodec_free_context(&avCodecCtx);
	av_frame_free(&frame);
	av_frame_free(&frame2);
	sws_freeContext(sws_ctx);
	av_packet_free(&avpacket);
}

void FFmpeg::timerEvent(QTimerEvent *event)
{
	if (av_read_frame(avf, avpacket) >= 0)
	{
		if (avpacket->stream_index == streamIndex) {
			avcodec_send_packet(avCodecCtx, avpacket);
			while (avcodec_receive_frame(avCodecCtx, frame) == 0)
			{
				sws_scale(sws_ctx, (const uint8_t * const *)frame->data, frame->linesize, 0, avCodecCtx->height, frame2->data, frame2->linesize);
				
				emit newFrame(frame2);
			}
		}
		av_packet_unref(avpacket);
	}

	QObject::timerEvent(event);
}

// 垂直翻转函数
void FFmpeg::flip_frame_vertical(AVFrame *frame) {
	int linesize = frame->linesize[0];
	uint8_t *tmp = (uint8_t *)av_malloc(linesize);
	int height = frame->height;
	int width = 3*frame->width;

	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width; x++) {
			int src_index = y * linesize + x;
			int dst_index = (height - 1 - y) * linesize + x;
			// 交换像素数据
			tmp[0] = frame->data[0][src_index];
			frame->data[0][src_index] = frame->data[0][dst_index];
			frame->data[0][dst_index] = tmp[0];
		}
	}
	av_free(tmp);
}
