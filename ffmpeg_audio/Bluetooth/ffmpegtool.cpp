#include "ffmpegtool.h"

#include <QDebug>
#include <QFile>

#define MAX_AUDIO_FRAME_SIZE 80960 // 一帧音频最大长度（样本数），该值不能太小

FFmpegTool::FFmpegTool(QObject *parent)
{

}

void FFmpegTool::run()
{
    //ffmpeg();
    normalData();
}
void FFmpegTool::normalData()
{
    QFile file("audio_data_.raw");
    if(!file.open(QIODevice::ReadOnly ))
    {
        qDebug()<<"file open error";
        return;
    }
    int sampleRate = 44100;
    int channels = 2;
    //int sampleFMT = AV_SAMPLE_FMT_S16;
    int frameSize = sampleRate * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    quint8 *buffer = (quint8*)av_malloc(frameSize);
    if(!buffer)
    {
        qDebug()<<"av_malloc error";
        return;
    }
    QByteArray arr = file.readAll();
    if(arr.size() != frameSize)
    {
        qDebug()<<"file.read error";
        return;
    }
}

void FFmpegTool::ffmpeg()
{
    AVFormatContext *avFormatCtx = nullptr;
    int ret = avformat_open_input(&avFormatCtx, "audio_data_.raw", NULL, NULL);
    if(ret != 0)
    {
        qDebug()<<"avformat_open_input error";
        return;
    }

    ret = avformat_find_stream_info(avFormatCtx, NULL);
    if(ret < 0)
    {
        qDebug()<<"avformat_find_stream_info error";
        return;
    }

    int audioIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1,NULL, 0);
    if(audioIndex < 0)
    {
        qDebug()<<"av_find_best_stream error";
        return;
    }

    AVStream *stream = avFormatCtx->streams[audioIndex];

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if(!codec)
    {
        qDebug()<<"avcodec_find_decoder error";
        return;
    }

    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx)
    {
        qDebug()<<"avcodec_alloc_context3 error";
        return;
    }

    //将音频流中的编解码参数，复制给解码器
    avcodec_parameters_to_context(codecCtx, stream->codecpar);
    ret = avcodec_open2(codecCtx, codec, NULL);
    if(ret != 0)
    {
        qDebug()<<"avcodec_open2 error";
        return;
    }

    //
    AVChannelLayout outChanLayout = codecCtx->ch_layout;//声道布局
    enum AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;//多少位，有无符号，大端小端
    int outSampleRate = codecCtx->sample_rate;//采样率
    int outNbSample = codecCtx->frame_size;//采样数量
    if(outNbSample <= 0)
        outNbSample = 512;
    int outChannels = outChanLayout.nb_channels;//声道数量

    //输出缓冲区大小
    int outBufferSize = av_samples_get_buffer_size(NULL, outChannels, outNbSample,outSampleFmt, 1);
    //分配输出缓冲区空间
    unsigned char *outBuffer = (unsigned char*)av_malloc(MAX_AUDIO_FRAME_SIZE * outChannels);


    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    //轮询数据包
    while(av_read_frame(avFormatCtx, packet) >= 0)
    {
        if(packet->stream_index == audioIndex)
        {
            //把未解压的数据包发给解码器实例
            ret = avcodec_send_packet(codecCtx, packet);
            if(0 == ret)
            {
                //从解码器实例获取还原后的数据帧
                ret = avcodec_receive_frame(codecCtx, frame);
                if(0 == ret)
                {
                    QByteArray arr((const char*)(*frame->data));
                    qDebug()<<arr.size()<<":" <<arr.toHex(' ');
                }
            }
        }
    }
}
