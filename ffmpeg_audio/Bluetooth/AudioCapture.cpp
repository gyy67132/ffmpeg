#include "audiocapture.h"

#include <QtConcurrent>

AudioCapture::AudioCapture()
{


}

AudioCapture::~AudioCapture()
{

}

void AudioCapture::startCaputer()
{
    QAudioFormat format;
    // 设置采样率
    format.setSampleRate(44100);
    // 设置声道数
    format.setChannelCount(2); // 单声道
    // 设置样本大小
    format.setSampleSize(16);
    // 设置样本类型
    format.setCodec("audio/pcm");
    // 设置字节顺序
    format.setByteOrder(QAudioFormat::LittleEndian);
    // 设置样本类型
    format.setSampleType(QAudioFormat::SignedInt);

    // 检查音频格式是否支持
    if (!QAudioDeviceInfo::availableDevices(QAudio::AudioInput).isEmpty()) {
        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(format)) {
            qWarning() << "Default format not supported, trying to use nearest.";
            format = info.nearestFormat(format);
        }
    }

    file = new QFile;
    audio = new QAudioInput(format, this);

//    QBuffer* buffer = new QBuffer();
//    buffer->open(QIODevice::WriteOnly);
//    //连接状态改变信号
//    connect(audio, &QAudioInput::stateChanged, this, [this, buffer](QAudio::State state){
//        if (state == QAudio::ActiveState) {
//            // 开始读取音频数据
//            //audio->start(buffer);
//            // 这里可以设置一个定时器来周期性地从buffer中读取数据
//        } else if (state == QAudio::StoppedState) {
//            // 停止捕获
//            buffer->close();
//            // 处理或保存buffer中的数据
//        }
//    });

    file->setFileName("audio_data_.raw");
    file->open(QIODevice::WriteOnly | QIODevice::Truncate);

    audio->start(file);
}


void AudioCapture::stopCapture()
{
    audio->stop();
    file->close();
    delete audio;
    delete file;
    audio = nullptr;
    file = nullptr;
    //exit(1);
}
