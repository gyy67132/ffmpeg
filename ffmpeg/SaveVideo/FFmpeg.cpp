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
	frame2->format = AV_PIX_FMT_YUV420P;
	av_frame_get_buffer(frame2, 1);

	sws_ctx = sws_getContext(avCodecCtx->width, avCodecCtx->height, avCodecCtx->pix_fmt, avCodecCtx->width, avCodecCtx->height, 
		AV_PIX_FMT_YUV420P,SWS_BILINEAR, NULL, NULL, NULL);
	avpacket = av_packet_alloc();
	
	/// <summary>
	/// /////////
	/// </summary>
	/// <param name="parent"></param>
	const char* out_filename = "./ggy.mp4";
	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if (!ofmt_ctx)
		return;
	ofmt = NULL;
	ofmt = ofmt_ctx->oformat;

	streamCount = avf->nb_streams;
	stream_mapping = (int*)av_calloc(streamCount, sizeof(*stream_mapping));
	if (!stream_mapping)
		return;

	int streamIndex2 = 0;
	for (int i = 0; i < avf->nb_streams; i++)
	{
		AVStream* out_stream;
		AVStream* in_stream = avf->streams[i];
		AVCodecParameters* in_codecpar = in_stream->codecpar;

		if (in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
			in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
			in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
		{
			stream_mapping[i] = -1;
			continue;
		}
		stream_mapping[i] = streamIndex2++;

		out_stream = avformat_new_stream(ofmt_ctx, NULL);

		if (!out_stream)
			return;
		if (avcodec_parameters_copy(out_stream->codecpar, in_codecpar) < 0)
			return;
		out_stream->codecpar->codec_tag = 0;
	}

	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0)
			return;
	}

	if (avformat_write_header(ofmt_ctx, NULL) < 0)
		return;

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

	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_closep(&ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);

	av_freep(&stream_mapping);
}

void FFmpeg::timerEvent(QTimerEvent *event)
{
	if (av_read_frame(avf, avpacket) >= 0)
	{
		if (avpacket->stream_index < streamCount && stream_mapping[avpacket->stream_index] >= 0) {
			
			AVStream* in_stream;
			AVStream* out_stream;
			in_stream = avf->streams[avpacket->stream_index];

			avpacket->stream_index = stream_mapping[avpacket->stream_index];
			out_stream = ofmt_ctx->streams[avpacket->stream_index];

			av_packet_rescale_ts(avpacket, in_stream->time_base, out_stream->time_base);
			avpacket->pos = -1;

			int ret = av_interleaved_write_frame(ofmt_ctx, avpacket);
		}
		av_packet_unref(avpacket);
	}
	else {
		av_write_trailer(ofmt_ctx);
	}
	QObject::timerEvent(event);
}