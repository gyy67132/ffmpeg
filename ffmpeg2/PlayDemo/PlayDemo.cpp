#include "PlayDemo.h"

extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

#include <QDebug>
#include <QImage>

AVFormatContext* fmt_ctx = NULL;
const AVCodec* codec = NULL;
AVCodecContext* codec_ctx = NULL;
AVFrame* frame = NULL;
AVPacket* packet = NULL;
int video_stream_index = -1;
struct SwsContext* sws_ctx = nullptr;
int iheight = 0;
int iwidth = 0;
int dstLinesize;
uint8_t* dstData = nullptr;
QImage image;

PlayDemo::PlayDemo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    const char* input_filename = "11.mp4";

    //打开输入文件并获取流信息
    if (avformat_open_input(&fmt_ctx, input_filename, NULL, NULL)) {
        qDebug() << "avformat_open_input error\n";
        return;
    }
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        qDebug() << "avformat_find_stream_info error\n";
        cleanUp();
        return;
    }
    
    //查找流视频
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1)
    {
        qDebug() << "find video_stream_index error\n";
        cleanUp();
        return;
    }
    
    //查找解码器
    codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    if (!codec)
    {
        qDebug() << "avcodec_find_decoder error\n";
        cleanUp();
        return;
    }

    //分配解码器上下文，并配置参数
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        qDebug() << "avcodec_alloc_context3 error\n";
        cleanUp();
        return;
    }
    if (avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar) < 0)
    {
        cleanUp();
        qDebug() << "avcodec_parameters_to_context error\n";
        return;
    }

    //打开解码器
    if (avcodec_open2(codec_ctx, codec, NULL) != 0)
    {
        cleanUp();
        qDebug() << "avcodec_open2 error\n";
        return;
    }

    //分配帧和包
    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet)
    {
        cleanUp();
        qDebug() << "av_frame_alloc or av_packet_alloc error\n";
        return;
    }
    AVStream* stream = fmt_ctx->streams[video_stream_index];
    iwidth = fmt_ctx->streams[video_stream_index]->codecpar->width;
    iheight = fmt_ctx->streams[video_stream_index]->codecpar->height;
    sws_ctx = sws_getContext(iwidth, iheight, (AVPixelFormat)fmt_ctx->streams[video_stream_index]->codecpar->format, iwidth, iheight, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

    /*int ret = av_image_alloc(&dstData, &dstLinesize, iwidth, iheight, AV_PIX_FMT_RGB24, 1);
    if (ret < 0)
    {
        cleanUp();
        qDebug() << "av_image_alloc error\n";
        return;
    }*/
    image = QImage(iwidth, iheight, QImage::Format_RGB888);
    dstData = image.bits();
    dstLinesize = image.bytesPerLine();
    startTimer(30);
}

PlayDemo::~PlayDemo()
{
    cleanUp();
}

void PlayDemo::cleanUp()
{
    /*if (dstData)
        av_freep(dstData);*/
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if(frame)
        av_frame_free(&frame);
    if(packet)
        av_packet_free(&packet);
    if(codec_ctx)
        avcodec_free_context(&codec_ctx);
    if(fmt_ctx)
        avformat_close_input(&fmt_ctx);
}

void PlayDemo::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);

    int ret = 0;
    bool frame_decoded = false;

    while (!frame_decoded) {
        ret = av_read_frame(fmt_ctx, packet);
        if (ret < 0)
        {
            break;
        }
        if (packet->stream_index != video_stream_index)
        {
            av_packet_unref(packet);
            continue;
        }
            
        int ret = avcodec_send_packet(codec_ctx, packet);
        av_packet_unref(packet);
        if (ret != 0) {
            qDebug() << "avcodec_send_packet error\n";
            break;

        }
        while (ret >= 0) {
            ret = avcodec_receive_frame(codec_ctx, frame);
            if (ret == AVERROR(EAGAIN))
                break;
            else if (ret == AVERROR_EOF) {
                frame_decoded = true;
                break;
            }
            else if (ret < 0) {
                frame_decoded = true;
                break;
            }
            sws_scale(sws_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, iheight, &dstData, &dstLinesize);

            /*for (int y = 0; y < iheight; ++y) {
                memcpy(image.scanLine(y), dstData + y * dstLinesize, iwidth * 3);
            }*/
            ui.label->setPixmap(QPixmap::fromImage(image));

            frame_decoded = true;
            break;
            update();
        }
        av_packet_unref(packet);
    }
}
