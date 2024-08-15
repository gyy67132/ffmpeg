#ifndef BLUETOOTHDATA_H
#define BLUETOOTHDATA_H

#include <QObject>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
}


class BluetoothConnect;

class BluetoothData : public QObject
{
    Q_OBJECT
public:
    enum BluetoothState{
        None = 0,
        Command,
        SessisonID
    };
    static BluetoothData &instance(BluetoothConnect *b);
    void processData(QByteArray data);

private:
    void selectAudioData();

private:
    explicit BluetoothData(QObject *parent = nullptr, BluetoothConnect *b=nullptr);

    BluetoothConnect *bluetoothConnect;
    QByteArray audioByteArr;
    quint32 sessionID;
    bool recvAudioData;
    quint32 dataIndex;
    QByteArray sessionArr;

    AVCodecContext *codecCtx;
};

#endif // BLUETOOTHDATA_H
