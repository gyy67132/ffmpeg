#include "bluetoothconnect.h"

#include "globalmessage.h"
#include "BluetoothData.h"

#include <QThread>
#include <QTimer>
#include <QTextCodec>
#include <QDate>
#include <QTimeZone>
#include <QtEndian>
#include "ffmpegtool.h"


#define NOTIFY_CHARAC "{00002bb0-0000-1000-8000-00805f9b34fb}"
#define WRITE_CHARAC "{00002bb1-0000-1000-8000-00805f9b34fb}"
#define TOKEN "1234567890000000"


quint32 crcLookup[] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};

BluetoothConnect::BluetoothConnect(const QBluetoothDeviceInfo& info, QObject *parent)
    : QObject{parent},bluetoothDeviceInfo{info}
{
        // 只保存当前选中设备的服务uuid，删除其他
        //m_uuidList.clear();
        connect(this, &BluetoothConnect::sigDebugLog, &GlobalMessage::instance(), &GlobalMessage::bluetoothInfo);
        //创建蓝牙控制器对象
        m_control = QLowEnergyController::createCentral(info);
        if (m_control == NULL)
        {
            emit sigDebugLog(info.name() +" " + info.address().toString() + "bluetooth Controller not created");
        }
        else
        {
            emit sigDebugLog(info.name() +" " + info.address().toString() + "bluetooth Controller created");
        }

        connect(m_control, SIGNAL(connected()), this, SLOT(onControlConnected()));
        connect(m_control, SIGNAL(disconnected()), this, SLOT(onControlDisconnected()));
        connect(m_control, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(onServiceDiscovered(QBluetoothUuid)));
        connect(m_control, SIGNAL(discoveryFinished()), this, SLOT(onControlDiscoveryFinished()));
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(onControlError(QLowEnergyController::Error)));
        // 开始计时控制器的连接超时
        //m_controlTimer.start(10000);
        // 发起连接
        QTimer::singleShot(1000, this, [this](){
            m_control->connectToDevice();
        });

}

void BluetoothConnect::onControlConnected()
{
    // 停止连接计时
    //m_controlTimer.stop();
    emit sigDebugLog(bluetoothDeviceInfo.name() + QString::fromLocal8Bit("蓝牙控制器连接成功，开始搜索蓝牙服务！"));
    //m_uuidList.clear();
    QThread::msleep(1000);
    //搜索服务
    m_control->discoverServices();
}


void BluetoothConnect::onControlDisconnected()
{
    emit sigDebugLog("bluetooth connect failed");
}

void BluetoothConnect::onServiceDiscovered(const QBluetoothUuid &newService)
{
    emit sigDebugLog(QString::fromLocal8Bit("发现一个蓝牙uuid:").append(newService.toString()));
    m_uuidList.append(newService);
}

void BluetoothConnect::onControlDiscoveryFinished()
{
    emit sigDebugLog(bluetoothDeviceInfo.name() + QString::fromLocal8Bit("蓝牙服务搜索结束"));
    QThread::msleep(1000);
    createService(0);
}
void BluetoothConnect::onControlError(QLowEnergyController::Error value)
{
    emit sigDebugLog(bluetoothDeviceInfo.name() +QString::fromLocal8Bit("蓝牙服务搜索失败") + QString::number(value));
}


void BluetoothConnect::createService(int iRow)
{
    // 创建服务
    QBluetoothUuid btUuid;
    for(QBluetoothUuid uuid : m_uuidList)
        if(uuid.toString() == "{00001910-0000-1000-8000-00805f9b34fb}")
        {
            btUuid = uuid;
        }
    if(btUuid.isNull())
    {
        emit sigDebugLog("{00001910-0000-1000-8000-00805f9b34fb} not find");
        return;
    }
    m_service = m_control->createServiceObject(btUuid,this);
    if (m_service == NULL)
    {
        emit sigDebugLog(bluetoothDeviceInfo.name() +QString::fromLocal8Bit("创建蓝牙服务失败"));
    } else {
        emit sigDebugLog(bluetoothDeviceInfo.name() +QString::fromLocal8Bit("创建蓝牙服务成功"));

        //监听服务状态变化,有服务来
        connect(m_service,SIGNAL(stateChanged(QLowEnergyService::ServiceState))
                ,this,SLOT(onServiceStateChanged(QLowEnergyService::ServiceState)));
        //服务的characteristic变化，有数据传来
        connect(m_service,SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray))
                ,this,SLOT(onServiceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        //描述符成功被写
        connect(m_service,SIGNAL(descriptorWritten(QLowEnergyDescriptor,QByteArray))
                ,this,SLOT(onDescriptorWritten(QLowEnergyDescriptor,QByteArray)));
        //
        connect(m_service, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(onServiceError(QLowEnergyService::ServiceError)));
        connect(m_service, &QLowEnergyService::characteristicWritten, this, &BluetoothConnect::onCharacteristicWritten);//连接写入操作完成的信号
        connect(m_service, &QLowEnergyService::characteristicRead, this, &BluetoothConnect::onCharacteristicRead);//读
        connect(m_service, &QLowEnergyService::descriptorRead, this, &BluetoothConnect::onDescriptorRead);//读

        QThread::msleep(1000);
        //服务详情发现函数
        m_service->discoverDetails();
    }
}
void BluetoothConnect::onServiceError(QLowEnergyService::ServiceError value)
{
    emit sigDebugLog(bluetoothDeviceInfo.name() + "ServiceError " + QString::number(value));
}

void BluetoothConnect::onServiceStateChanged(QLowEnergyService::ServiceState newState)
{
    Q_UNUSED(newState);

    //发现服务
    if(m_service->state() == QLowEnergyService::ServiceDiscovered){
        QList<QLowEnergyCharacteristic> m_charList = m_service->characteristics();
        for(int i=0; i<m_charList.size(); i++)
        {
            QLowEnergyCharacteristic bleCharacteristic = m_charList.at(i);
            if(bleCharacteristic.isValid())
            {
                emit sigDebugLog(bluetoothDeviceInfo.name() + QString::fromLocal8Bit("发现服务") + bleCharacteristic.uuid().toString());

                //描述符定义特征如何由特定客户端配置
                m_descriptor = bleCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                if(m_descriptor.isValid())
                {
                    m_service->writeDescriptor(m_descriptor,QByteArray::fromHex("0100"));
                }
                if (bleCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse || bleCharacteristic.properties() & QLowEnergyCharacteristic::Write)
                {
                    m_writeCharacteristic = bleCharacteristic;
                }

            }
            // bleCharacteristic = m_charList.at(i);
            // if(bleCharacteristic.isValid())
            // {
            //     emit sigDebugLog(bluetoothDeviceInfo.name() + QString::fromLocal8Bit("发现服务") + bleCharacteristic.uuid().toString());
            //     //if("{00002bb0-0000-1000-8000-00805f9b34fb}" != bleCharacteristic.uuid().toString())
            //         //continue;
            //     //描述符定义特征如何由特定客户端配置
            //     m_descriptor = bleCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

            //     if(m_descriptor.isValid())
            //     {
            //         emit sigDebugLog(bleCharacteristic.uuid().toString() + "write 0100\n");
            //         m_service->writeDescriptor(m_descriptor,QByteArray::fromHex("0100"));
            //     }
            //     QLowEnergyCharacteristic t = m_service->characteristic(bleCharacteristic.uuid());
            //     m_service->readCharacteristic(bleCharacteristic);
            //     int a = bleCharacteristic.properties();
            //     emit sigDebugLog(bleCharacteristic.uuid().toString() + QString::number(a));
            //     if (bleCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse || bleCharacteristic.properties() & QLowEnergyCharacteristic::Write)
            //     {
            //         m_character = bleCharacteristic;
            //     }

            // }
        }
    }
}


void BluetoothConnect::onServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray newValue)
{
    int len = newValue.size();
    if(len > 0){
        emit sigDebugLog(QString::fromLocal8Bit("接收数据rec:").append(newValue.toHex()));
        ParserReceiveData(newValue);
        //BluetoothData::instance(this).processData(newValue);
    }

}

void BluetoothConnect::onCharacteristicWritten(const QLowEnergyCharacteristic &info,
                                               const QByteArray &value)
{
    emit sigDebugLog("Characteristic Written" + value);
}
void BluetoothConnect::onDescriptorWritten(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    //描述符写入成功，启用通知
    emit sigDebugLog("Descriptor Written" + value);
    //第一次握手
    sendCmd(QByteArray::fromHex("010100010100"));
}


void BluetoothConnect::onCharacteristicRead(const QLowEnergyCharacteristic &info,
                                            const QByteArray &value)
{
    //int len = value.size();
    emit sigDebugLog(info.uuid().toString() + "Characteristic Read:" /*+ QString::number(len) */+ value.toHex(' '));
    //emit sigDebugLog(info.uuid().toString() + "writeDescriptor:010100010100");
    // m_service->writeDescriptor(m_descriptor, QByteArray::fromHex("010100010100"));
    // sendCmd(QByteArray::fromHex("010100010100"));
}

void BluetoothConnect::onDescriptorRead(const QLowEnergyDescriptor &info,
                                        const QByteArray &value)
{
    //int len = value.size();
    emit sigDebugLog(info.uuid().toString() + "Descriptor Read:" /*+ QString::number(len) */+ value.toHex(' '));    
}
void BluetoothConnect::sendCmd(const QByteArray &data)
{
    if (m_service && m_writeCharacteristic.isValid())
    {
        emit sigDebugLog("send cmd:" + data.toHex(' '));
        m_service->writeCharacteristic(m_writeCharacteristic, data, QLowEnergyService::WriteWithResponse);
    }
}


void BluetoothConnect::ParserReceiveData(QByteArray data)
{
    switch(data[0])
    {
    case 1:
        ReceiveCmd(data);
        break;
    case 2:
        ReceiveVoice(data);
        break;
    case 3:
        //ota
        break;
    case 4:
        //drt
        break;
    default:
        break;
    }
}

void BluetoothConnect::ParserStartRecordingParam(QByteArray data)
{
    // 4 Byte
    QByteArray sessionIdByte = data.mid(0,4);
    // 1 Byte
    QByteArray scene = data.mid(4,1);

    /**
     * 4 Byte
     * 当前录音的起始位置
    */
    QByteArray start = data.mid(5,4);
    /**
     * 1 Byte
     * 0 成功;
     * >0 失败——作为响应 APP 请求时 有意义
     * 1. 空间已满; 2. U 盘模式; 3. 硬件异常;4、当 前正忙，稍后重试;5. wifi 模式 6.录音已停止 7.等待绑定
    */
    QByteArray status = data.mid(9,1);
    uint sessionId = 0;

    qDebug() << "sessionId:" << sessionIdByte.toHex() << ",scene:" <<scene.toHex() << ",start:" <<start.toHex()<<",status:"<<status.toHex();
    switch (status.toInt())
    {
    case 0:
        // Reset voice size & byte buffer offset
        m_nVoicefileSize = 0;
        m_nVoiceByteoffset = 0;

       // Create converter.
       // converter = OpusSaveToOgg("output.ogg");
       //    // Write ogg file header.
       //    converter?.opusSaveOggHeader()
        m_pFile = new QFile;
        m_pFile->setFileName("output.opus");
        if (!m_pFile->open(QIODevice::WriteOnly))
        {
            emit GlobalMessage::instance().sendMessage("Cannot open file for writing output.pcm");
            return;
        }

       // // Request opus data.
        //sessionId = sessionIdByte[0] + (sessionIdByte[1] << 8) +  (sessionIdByte[2] << 16)+ (sessionIdByte[3] << 24) ;
        RequestVoiceData(sessionIdByte, m_nVoiceByteoffset);

      break;
    default:
      break;
    }


}

void BluetoothConnect::ParserEndRecordingParam(QByteArray data)
{
    QByteArray filesize = data.mid(6,4);
    qDebug() <<"data:"<< data.toHex() <<",filesize:" << filesize.toHex();
    // Update voice size.
    m_nVoicefileSize = int(((filesize[3] & 0xFF) << 24) + ((filesize[2] & 0xFF) << 16) + ((filesize[1] & 0xFF) << 8) + ((filesize[0] & 0xFF) << 0));
}

void BluetoothConnect::ReceiveCmd(QByteArray data)
{
    if(data.size() < 3)
        return;
    int cmd = int(((data[2] & 0xFF) << 8) + ((data[1] & 0xFF) << 0));
    QByteArray param = data.mid(3);
    switch(cmd)
    {
    case 1:
        //第三次握手：同步时区
        sendCmd(GetTimeZone());
        break;
    case 2:
        //第二次握手：发token
        sendCmd(GetToken());
        break;
    case 3:
        break;
    case 4:
        break;
    case 15:
        //仓上按键唤醒类型
        break;
    case 20:
        //开始录音
        ParserStartRecordingParam(param);
        break;
    case 23:
        //结束录音
        ParserEndRecordingParam(param);
        break;
    default:
        break;
    }
}

void BluetoothConnect::ReceiveVoice(QByteArray data)
{
    qDebug()<<data.toHex(' ');

    //解析录音
    QByteArray type = data.mid(0,1);
    if(type.toHex() != "02")
    {
        return;
    }
    //最小长度判断
    if(data.count() < 90)
    {
        return;
    }
    // 4 Byte
    QByteArray sessionId = data.mid(1,4);
    // 4 Byte
    QByteArray offset = data.mid(5,4);
    // 1 Byte
    QByteArray length = data.mid(9,1);
    int nLen = length[0];
    // 总长度是否匹配
    int totalLength = 10 + nLen;
    if(data.count() != totalLength)
    {
        return;
    }
    QByteArray content = data.mid(10);
    bool isLast = false;

    QByteArray byte("ffffffff");
    qDebug() << offset.toHex() << " " << byte;
    if(offset.toHex().toLower() == byte.toLower())
    {
        isLast = true;
    }

    if(isLast)
    {
        // 一次请求返回的最后一包数据为空包
        // 戒鼠仓按键结束录音后，会设置文件大小，偏移量大于或等于文件大小时，结束流程
        qDebug() << "m_nVoicefileSize:" << m_nVoicefileSize << ",m_nVoiceByteoffset:" << m_nVoiceByteoffset;
        if(m_nVoicefileSize != 0 && m_nVoiceByteoffset >= m_nVoicefileSize)
        {
            //录音结束
            if(m_pFile)
            {
                m_pFile->close();
                //转ogg
                // FFmpegTool *ffmpegTool= new FFmpegTool;
                // ffmpegTool->convertOpusToOgg("output.opus", "output.ogg");
            }
            return;
        }

        // Save opus data.
        if(m_pFile)
        {
            // 将QByteArray中的数据写入文件
            for (int n = 0; n < m_tmpArray.size(); n++)
            {
                int i = m_tmpArray[n] & 0xFF; // 获取无符号值
                if (i > 0x7F)
                {
                    i -= 0xFF + 1; // 调整值
                }
                // 写入调整后的值
                m_tmpArray[n] = i;
            }
            qint64 bytesWritten = m_pFile->write(m_tmpArray);
            if (bytesWritten == m_tmpArray.size())
            {
                emit GlobalMessage::instance().sendMessage("write audio bytes to file Success");
            }
            else
            {
                emit GlobalMessage::instance().sendMessage("write audio bytes to file Error");
            }
        }

        // Next request, get opus data.
        RequestVoiceData(sessionId, m_nVoiceByteoffset);
        return;
    }
    //保存录音数据
    m_tmpArray.append(content);
}

QByteArray BluetoothConnect::GetToken()
{
    QByteArray array;
    array.append("01");
    array.append("01");
    // deviceType
    array.append("00");
    // bleVersion
    array.append("01");
    // check pass
    array.append("01");
    array.append("01");
    // token
    QByteArray token = QString(TOKEN).toUtf8();
    array.append(token.toHex());
    //Long_audio
    array.append("00");
    // Device_token[8]:扫码绑定传过来的笔端token，如果非扫码绑定请写成8个0
    array.append("0000000000000000");
    //QString name = QString("背夹 c1 pro");

    array.append("0de8838ce5a4b92063312070726f");

    return QByteArray::fromHex(array);
}

QByteArray BluetoothConnect::GetTimeZone()
{
    QByteArray array;
    array.append("01");
    array.append("04");
    array.append("00");
    QDateTime currentDate = QDateTime::currentDateTimeUtc();
    uint timestamp = currentDate.toTime_t();

    QTimeZone systemTimeZone = QTimeZone::systemTimeZone();
    quint32 systemOffset = systemTimeZone.offsetFromUtc(QDateTime::currentDateTime());
    quint8 timezone = quint8(systemOffset/3600);

    timestamp -= timezone * 3600;
    qDebug() << timestamp << timezone;

    QByteArray offsetByte(reinterpret_cast<const char*>(&timestamp),sizeof(timestamp));
    array.append(offsetByte.toHex());
    QByteArray timezoneByte(reinterpret_cast<const char*>(&timezone),sizeof(timezone));
    array.append(timezoneByte.toHex());
    qDebug()<<array;
    return QByteArray::fromHex(array);
}

void BluetoothConnect::RequestVoiceData(QByteArray sessionId, quint32 start)
{
    //清空临时数组
    m_tmpArray.clear();
    m_nVoiceByteoffset += 1600;
    QByteArray array;
    array.append("01");
    array.append("1C");
    array.append("00");
    array.append(sessionId.toHex(),8);
    QByteArray startByte(reinterpret_cast<const char*>(&start),sizeof(start));
    array.append(startByte.toHex());
    QByteArray endByte(reinterpret_cast<const char*>(&m_nVoiceByteoffset),sizeof(m_nVoiceByteoffset));
    array.append(endByte.toHex());
    array.append("00");
    qDebug()<<array;

    sendCmd(QByteArray::fromHex(array));
}

void BluetoothConnect::OpusSaveOggHeader()
{

}

// 计算 OGG 页面校验和
QByteArray OggPageChecksum(QByteArray page,int length)
{
    quint32 crcReg = 0;
    int i = 0;
    while(i < length)
    {
        quint32 t1 = (crcReg << 8) & 0xFFFFFFFF;
        quint32 t2 = (crcReg >> 24) & 0xff;
        quint32 t3 = t2 ^ (page[i] & 0xFF); // 注意：page[i] 是带符号的，需要转换为无符号
        crcReg = t1 ^ crcLookup[t3];
    }
    QByteArray array(sizeof(crcReg),0);
    char *data = array.data();
    qToLittleEndian(crcReg,data);// 将整数转换为小端形式并存储到数组中
    return array;
}

// 创建 P0 Ogg 头结构
// QByteArray CreateP0OggHeadStruct(quint32 snID)
// {
//     // 初始化字节数组
//     QByteArray initialBytes = {79, 103, 103, 83, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 210, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// }
