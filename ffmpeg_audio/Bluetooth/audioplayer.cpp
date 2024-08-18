#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject{parent}
{

}

void AudioPlayer::Play()
{
    QFile *sourceFile = new QFile;   // class member.
    QAudioOutput* audio; // class member.
    {
        sourceFile->setFileName("audio_data_.raw");
        sourceFile->open(QIODevice::ReadOnly);


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

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format)) {
            qWarning() << "Raw audio format not supported by backend, cannot play audio.";
            return;
        }

        audio = new QAudioOutput(format, this);
        connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
        audio->start(sourceFile);
    }
}
