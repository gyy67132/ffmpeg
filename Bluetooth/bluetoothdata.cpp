#include "bluetoothdata.h"

#include "globalmessage.h"
#include "bluetoothconnect.h"
#include "bufferutil.h"

#include <QBuffer>
#include <QFile>
#include <QtEndian>

BluetoothData::BluetoothData(QObject *parent, BluetoothConnect *b)
    : QObject{parent},bluetoothConnect{b}
{

    dataIndex = 0;

    AVFormatContext *avFormatCtx =nullptr;
    avformat_alloc_output_context2(&avFormatCtx, NULL, NULL, "audio.ogg");
    if(!avFormatCtx)
    {
        qDebug("创建输出上下文失败\n");
        return;
    }

    const AVOutputFormat *ofmt = av_guess_format("ogg",NULL, NULL);
    if(!ofmt)
    {
        qDebug("查找不到输出格式\n");
        return;
    }

    avFormatCtx->oformat = ofmt;
    //->filename

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_VORBIS);
    if(!codec)
    {
        qDebug("VORBIS is not find\n");
    }

    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx)
    {
        qDebug("could not allocate codec context\n");
    }
}

BluetoothData& BluetoothData::instance(BluetoothConnect *b)
{
    static BluetoothData g_BluetoothData = BluetoothData(nullptr, b);
    return g_BluetoothData;
}

/*

接收数据
rec:010f0002
接收数据
rec:011400cb5bb566010000000000
接收数据
rec:011700cb5bb566010140da0300
*/

void BluetoothData::processData(QByteArray data)
{
    int len = data.size();
    if(4 == len)
    {

    }else if(13 == len)
    {
        if((char)0x14 == data[1])
        {
            sessionArr = data.mid(3, 4);
            sessionID = data[3] + (data[4] << 8) +  (data[5] << 16)+ (data[6] << 24) ;
            //sessionID = data[3] << 24 + (data[4] << 16) +  (data[5] << 8) + (data[6]) ;
            emit GlobalMessage::instance().sendMessage("record audio start sessionId:" + QString::number(sessionID));
            if(sessionID > 0){
                //recvAudioData = true;
                audioByteArr.clear();
                selectAudioData();
            }

        }else if((char)0x17 == data[1])//录音结束
        {
            //recvAudioData = false;
            //emit GlobalMessage::instance().sendMessage("record audio stop sessionId:" + QString::number(sessionID));

            QFile file("output.pcm");
            if (!file.open(QIODevice::WriteOnly)) {
                emit GlobalMessage::instance().sendMessage("Cannot open file for writing output.wav");
                return;
            }

            // 将QByteArray中的数据写入文件
            qint64 bytesWritten = file.write(audioByteArr);
            if (bytesWritten == audioByteArr.size()) {
                 emit GlobalMessage::instance().sendMessage("write audio all bytes to file Success");
            }else{
                emit GlobalMessage::instance().sendMessage("write audio all bytes to file Error");
            }
            file.close();
        }
    }else if(90 == len)
    {
        if(data.mid(5, 4).toHex() == "ffffffff")//单次结束
        {
            selectAudioData();
            emit GlobalMessage::instance().sendMessage("单次结束 index:" + QString::number(dataIndex));
            return;
        }
        audioByteArr.append(data.mid(10));
    }
}

void BluetoothData::selectAudioData()
{
    //while(recvAudioData)
    {
        QByteArray cmd;
        cmd.append((char)1);

        cmd.append((char)28);
        cmd.append((char)0);

         QByteArray sessionByte;
         sessionByte.resize(4);
        // sessionByte[0] = sessionID >> 24;
        // sessionByte[1] = (sessionID >> 16 & 0x00ff);
        // sessionByte[2] = (sessionID >> 8 & 0x0000ff);
        // sessionByte[3] = (sessionID & 0x000000ff);
        // sessionByte[0] = (char)(sessionID & 0x000000ff);
        // sessionByte[1] = (char)((sessionID & 0x0000ff00)>>8);
        // sessionByte[2] = (char)((sessionID & 0x00ff0000)>>16);
        // sessionByte[3] = (char)((sessionID >>24 )&0xff);
        cmd.append(sessionArr);


        QByteArray startArr;
        startArr.resize(4);
        quint32 value = dataIndex * 1600;
        startArr[0] =  (char)(value & 0x000000ff);
        startArr[1] = (char)((value & 0x0000ff00) >> 8);
        startArr[2] = (char)((value & 0x00ff0000) >> 16);
        startArr[3] = (char)((value & 0xff000000) >> 24);
        cmd.append(startArr);


        QByteArray endArr;
        endArr.resize(4);
        quint32 value2 = (dataIndex + 1) * 1600;
        endArr[0] = (char)(value2 & 0x000000ff);
        endArr[1] = (char)((value2 & 0x0000ff00) >> 8);
        endArr[2] =(char) ((value2 & 0x00ff0000) >> 16);
        endArr[3] = (unsigned char)((value2 & 0xff000000) >> 24);
        cmd.append(endArr);

        cmd.append(static_cast<char>(0));

        //
        // cmd.clear();
        // QBuffer buffer(&cmd);
        // buffer.open(QBuffer::WriteOnly);
        // buffer.putChar(1);

        // BufferUtil::write16(buffer, 28);

        // BufferUtil::write32(buffer, sessionID);
        // BufferUtil::write32(buffer, 0);
        // BufferUtil::write32(buffer, (dataIndex + 1) * 1600);
        // buffer.putChar(0);
        // buffer.close();
        //audioByteArr.clear();
        //cmd.clear();
        //cmd = QByteArray::fromHex("011c003aeaa766000000004006000000");
        dataIndex++;
        //emit GlobalMessage::instance().sendMessage("index:" + QString::number(dataIndex));
        bluetoothConnect->sendCmd(cmd);
    }
}
