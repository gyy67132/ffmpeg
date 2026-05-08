#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QAudioOutput>
#include <QAudioFormat>
#include <QIODevice>
#include <QBuffer>
#include <QDebug>
#include <QTimer>
#include <QFile>

class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlayer(QObject *parent = nullptr);

    void Play();
    void Pause();
    void fastForward();

    QAudioOutput* audio;
signals:

private:

};

#endif // AUDIOPLAYER_H
