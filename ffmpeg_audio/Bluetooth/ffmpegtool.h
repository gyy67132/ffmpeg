#ifndef FFMPEGTOOL_H
#define FFMPEGTOOL_H

#include <QThread>

extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
}

class FFmpegTool : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegTool(QObject *parent = nullptr);


protected:
    virtual void run();

signals:

private:
    void ffmpeg();
    void normalData();
};

#endif // FFMPEGTOOL_H
