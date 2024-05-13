#pragma once

#include <QObject>

extern "C"
{
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libswscale/swscale.h"
}

class FFmpeg  : public QObject
{
	Q_OBJECT

public:
	FFmpeg(QObject *parent);
	~FFmpeg();
	static void flip_frame_vertical(AVFrame *frame);

protected:
	void timerEvent(QTimerEvent *event) override;

signals:
	void newFrame(AVFrame *);

private:
	AVFormatContext *avf = NULL;
	AVPacket *avpacket;
	AVCodecContext *avCodecCtx;
	AVFrame *frame;
	AVFrame *frame2;
	struct SwsContext *sws_ctx;
	int streamIndex;
};
