
extern "C"
{
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libavutil/opt.h"
	#include "libavutil/imgutils.h"
	#include "libswscale/swscale.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

int main()
{
	const char* path = "11.mp4";

	AVFormatContext* avFmtCtx = NULL;
	if(avformat_open_input(&avFmtCtx, path, NULL, NULL) != 0)
	{
		printf("avformat_open_input error");
		return -1;
	}

	if (avformat_find_stream_info(avFmtCtx, NULL) < 0)
	{
		printf("avformat_find_stream_info error");
		return -1;
	}

	int64_t time = avFmtCtx->duration;//Î¢Ãî 
	int mbittime = time / 1000000 / 60;//·Ö
	int mmintime = time / 1000000 % 60;
	printf("%d·Ö%dÃë", mbittime, mmintime);

	av_dump_format(avFmtCtx, 0, path, 0);

	int videoStream = av_find_best_stream(avFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	
	const AVCodec* vCodec = avcodec_find_decoder(avFmtCtx->streams[videoStream]->codecpar->codec_id);
	if (!vCodec)
	{
		printf("avcodec_find_decoder error");
		return -1;
	}

	AVCodecContext* avCodecCtx = avcodec_alloc_context3(vCodec);
	avcodec_parameters_to_context(avCodecCtx, avFmtCtx->streams[videoStream]->codecpar);
	if (0 != avcodec_open2(avCodecCtx, vCodec, NULL))
	{
		printf("avcodec_open2 error");
		return -1;
	}

	AVFrame* frame = av_frame_alloc();
	frame->width = avCodecCtx->width;
	frame->height = avCodecCtx->height;
	//frame->format = AV_PIX_FMT_YUV420P;
	int ret = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, avCodecCtx->width, avCodecCtx->height, 1);
	uint8_t* buff = (uint8_t*)malloc(ret);

	if (av_image_fill_arrays(frame->data, frame->linesize, buff, AV_PIX_FMT_YUV420P,
			avCodecCtx->width, avCodecCtx->height, 1) < 0)
	{
		printf("av_image_fill_arrays error");
		return -1;
	}
	// (frame, 1);

	AVPacket* packet = av_packet_alloc();//(AVPacket*)av_malloc(sizeof(AVPacket));

	AVFrame* frameYUV = av_frame_alloc();

	SwsContext* swsCtx = NULL;
	swsCtx = sws_getContext(avCodecCtx->width, avCodecCtx->height, avCodecCtx->pix_fmt,
		avCodecCtx->width, avCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
		NULL, NULL, NULL);

	int frameCount = 0;

	FILE* f1 = fopen("yuv.y", "wb+");
	FILE* f2 = fopen("yuv.u", "wb+");
	FILE* f3 = fopen("yuv.v", "wb+");

	while (av_read_frame(avFmtCtx, packet) == 0)
	{
		if (packet->stream_index == videoStream)
		{
			avcodec_send_packet(avCodecCtx, packet);
			if (0 == avcodec_send_packet(avCodecCtx, packet))
			{
				while (0 == avcodec_receive_frame(avCodecCtx, frameYUV))
				{
					sws_scale(swsCtx, (const uint8_t**)frameYUV->data, frameYUV->linesize, 0, 
								avCodecCtx->height
								, frame->data, frame->linesize);

					fwrite(frame->data[0], 1, avCodecCtx->width * avCodecCtx->height, f1);
					fwrite(frame->data[1], 1, avCodecCtx->width * avCodecCtx->height/4, f2);
					fwrite(frame->data[2], 1, avCodecCtx->width * avCodecCtx->height/4, f3);
					frameCount++;
					printf("frame index:%d\n", frameCount);
				}
			}
			else {
				printf("avcodec_send_packet error");
				//return -1;
			}
			
		}
		av_packet_unref(packet);
	}

	fclose(f1);
	fclose(f2);
	fclose(f3);

	sws_freeContext(swsCtx);
	av_frame_free(&frame);
	av_frame_free(&frameYUV);
	avformat_close_input(&avFmtCtx);
	return 0;
}