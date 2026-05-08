#include "bluetoothconnect.h"

#include "globalmessage.h"
#include "BluetoothData.h"

#include <QThread>
#include <QDateTime>
#include <QTimeZone>

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
        m_control->connectToDevice();
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
    QLowEnergyCharacteristic bleCharacteristic;
    //发现服务
    if(m_service->state() == QLowEnergyService::ServiceDiscovered){
        QList<QLowEnergyCharacteristic> m_charList = m_service->characteristics();
        for(int i=0; i<m_charList.size(); i++){
            bleCharacteristic = m_charList.at(i);
            if(bleCharacteristic.isValid()){
                emit sigDebugLog(bluetoothDeviceInfo.name() + QString::fromLocal8Bit("发现服务") + bleCharacteristic.uuid().toString());
                //if("{00002bb0-0000-1000-8000-00805f9b34fb}" != bleCharacteristic.uuid().toString())
                    //continue;
                  QLowEnergyDescriptor  m_descriptor = bleCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

                if(m_descriptor.isValid()){
                    emit sigDebugLog(bleCharacteristic.uuid().toString() + "write 0100\n");
                    m_service->writeDescriptor(m_descriptor,QByteArray::fromHex("0100"));
                }
                QLowEnergyCharacteristic t = m_service->characteristic(bleCharacteristic.uuid());
                m_service->readCharacteristic(bleCharacteristic);
                int a = bleCharacteristic.properties();
                emit sigDebugLog(bleCharacteristic.uuid().toString() + QString::number(a));
                if (bleCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse || bleCharacteristic.properties() & QLowEnergyCharacteristic::Write)
                {
                    m_character = bleCharacteristic;
                }

            }
        }
    }
}


void BluetoothConnect::onServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray newValue)
{
    int len = newValue.size();
    if(len > 0){
        emit sigDebugLog(QString::fromLocal8Bit("接收数据rec:").append(newValue.toHex()));
        BluetoothData::instance(this).processData(newValue);
        //ReceiveCmd(newValue);
    }
}

#define TOKEN "1234567890000000"
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

void BluetoothConnect::ReceiveCmd(QByteArray data)
{
    if(data.size() < 3)
        return;
    int cmd = int(((data[2] & 0xFF) << 8) + ((data[1] & 0xFF) << 0));
    QByteArray param = data.mid(3);
    qDebug()<<"ggy cmd:"<<cmd << data.toHex();
    switch(cmd)
    {
    case 1:
        //第三次握手：同步时区
        emit sigDebugLog(QString::fromLocal8Bit("第三次握手信息发送:"));
        sendCmd(GetTimeZone());
        break;
    case 2:
        //第二次握手：发token
        emit sigDebugLog(QString::fromLocal8Bit("第二次握手信息发送:"));
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
        //ParserStartRecordingParam(param);
        break;
    case 23:
        //结束录音
        //ParserEndRecordingParam(param);
        break;
    default:
        break;
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

    //emit sigDebugLog(QString::fromLocal8Bit("第一次握手信息发送:"));
    //sendCmd(QByteArray::fromHex("010100010100"));
}

void BluetoothConnect::onCharacteristicRead(const QLowEnergyCharacteristic &info,
                                            const QByteArray &value)
{
    int len = value.size();
    emit sigDebugLog(info.uuid().toString() + "Characteristic Read:" /*+ QString::number(len) */+ value.toHex(' '));

    //emit sigDebugLog(info.uuid().toString() + "writeDescriptor:010100010100");
    //m_service->writeDescriptor(m_descriptor, QByteArray::fromHex("010100010100"));
    //sendCmd(QByteArray::fromHex("010100010100"));
}

void BluetoothConnect::onDescriptorRead(const QLowEnergyDescriptor &info,
                                        const QByteArray &value)
{
    int len = value.size();
    emit sigDebugLog(info.uuid().toString() + "Descriptor Read:" /*+ QString::number(len) */+ value.toHex(' '));    
}
void BluetoothConnect::sendCmd(const QByteArray &data)
{
    if (m_service && m_character.isValid())
    {
        emit sigDebugLog("send cmd:" + data.toHex(' '));
        m_service->writeCharacteristic(m_character, data, QLowEnergyService::WriteWithResponse);
    }
}
