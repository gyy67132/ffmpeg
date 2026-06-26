#pragma once

#include <QTcpSocket>
#include <QLabel>
class glWidget;

extern "C"
{
#include "libavcodec/codec.h"
#include "libavcodec/packet.h"
#include "libavcodec/avcodec.h"   
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#pragma pack(push, 1)
struct StreamPacketHeader {
    uint64_t flags;
    //uint32_t timestamp;
    uint32_t payload_size;
};
#pragma pack(pop)

#define PACKET_FLAG_CONFIG (UINT64_C(1) << 63)
#define PACKET_FLAG_KEY_FRAME (UINT64_C(1) << 62)
#define PACKET_PTS_MASK (PACKET_FLAG_KEY_FRAME - 1)
class TcpSocket :
    public QTcpSocket
{
    Q_OBJECT
public:
    TcpSocket();
    bool readHeader(StreamPacketHeader* streamPacketHeader);
    bool readBody(uint8_t* buf, qint32 bufSize);
    bool readWidthHeight(int& width, int& height);
signals:
    void signal_updateImage(const QImage &img);
    void signal_updateFrame(AVFrame* frame);
public slots:
    void connectedServer();
    void updateImage(QImage image);
private:
    QLabel* label;
    glWidget* w;
};

