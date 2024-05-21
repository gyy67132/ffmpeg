#include "Recoder.h"


Recoder::Recoder()
{

}
Recoder::~Recoder()
{

}

bool Recoder::open()
{
	const AVCodec* inputCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!inputCodec)
		return false;
	void* opaque = Q_NULLPTR;
	const AVOutputFormat* outFormat = av_muxer_iterate(&opaque);
}

bool Recoder::write(AVPacket* packet)
{
	if (!headerWritten)
	{
		if (packet->pts != AV_NOPTS_VALUE)
			return false;
		AVStream *ostream = m
	}
}
