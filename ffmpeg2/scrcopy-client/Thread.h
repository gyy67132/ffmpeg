#pragma once

#include <QThread>

#include "TcpSocket.h"

extern "C"
{
    #include "libavcodec/codec.h"
    #include "libavcodec/packet.h"
    #include "libavcodec/avcodec.h"   
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
}

#define IMAGIN_W 1280
#define IMAGIN_H 720

class Thread :
    public QThread
{
    Q_OBJECT
public:
    Thread();
    ~Thread();
    void setSocket(TcpSocket* socket);
public slots:
    void doDecode();
signals:
    void signal_image(QImage);
    void signal_frame(AVFrame*);
protected:
    virtual void run();

private:
    TcpSocket* _socket;
    AVPacket* m_pending = nullptr;
    AVCodecParserContext* parser = nullptr;
    AVCodecContext* codecCtx = nullptr;
};

