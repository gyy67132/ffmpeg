#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QAudioInput>
#include <QAudioFormat>
#include <QIODevice>
#include <QBuffer>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QThread>

class AudioCapture : public QObject
{
    Q_OBJECT

public :
    AudioCapture();
    ~AudioCapture();
    QByteArray arrData;

    void startCaputer();
    void stopCapture();
protected:
    //virtual void run();
private:
    QFile *file;
    QAudioInput* audio;
};
#endif // AUDIOCAPTURE_H
