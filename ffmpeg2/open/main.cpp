
extern "C"
{
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libavutil/opt.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

int main()
{
	const char* path = "rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid";// "11.mp4";
	AVDictionary* opt = NULL;
	av_dict_set(&opt, "rtsp_transport", "tcp", 0);
	av_dict_set(&opt, "max_delay", "550", 0);
	
	avformat_network_init();

	AVFormatContext* avFmtCtx = NULL;
	if(avformat_open_input(&avFmtCtx, path, NULL, &opt) != 0)
	{
		printf("avformat_open_input error");
		return -1;
	}

	if (avformat_find_stream_info(avFmtCtx, NULL) < 0)
	{
		printf("avformat_find_stream_info error");
		return -1;
	}

	int64_t time = avFmtCtx->duration;//Œ¢√Ó 
	int mbittime = time / 1000000 / 60;//∑÷
	int mmintime = time / 1000000 % 60;
	printf("%d∑÷%d√Î", mbittime, mmintime);

	av_dump_format(avFmtCtx, 0, path, 0);

	return 0;
}