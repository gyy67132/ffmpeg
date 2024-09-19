#ifndef BLUETOOTHCONNECT_H
#define BLUETOOTHCONNECT_H

#include <QObject>

#include <QLowEnergyController>
#include <QFile>

class BluetoothData;

class BluetoothConnect : public QObject
{
    Q_OBJECT
    QBluetoothDeviceInfo bluetoothDeviceInfo;
    QLowEnergyController *m_control;
    QList<QBluetoothUuid> m_uuidList;

    QLowEnergyService *m_service;

    QLowEnergyCharacteristic m_writeCharacteristic;

    friend BluetoothData;
public:
    explicit BluetoothConnect(const QBluetoothDeviceInfo& info, QObject *parent = nullptr);

private slots:
    void onControlConnected();
    void onControlDisconnected();
    void onServiceDiscovered(const QBluetoothUuid &newService);

    void onControlDiscoveryFinished();
    void onControlError(QLowEnergyController::Error value);

    void onServiceStateChanged(QLowEnergyService::ServiceState newState);
    void onServiceError(QLowEnergyService::ServiceError);
    void onServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray newValue);

    void onCharacteristicWritten(const QLowEnergyCharacteristic &info, const QByteArray &value);
    void onDescriptorWritten(const QLowEnergyDescriptor &d, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &info, const QByteArray &value);
    void onDescriptorRead(const QLowEnergyDescriptor &info, const QByteArray &value);
private:
    void createService(int iRow);

    void sendCmd(const QByteArray &data);

    void ParserReceiveData(QByteArray data);
    void ParserStartRecordingParam(QByteArray data);
    void ParserEndRecordingParam(QByteArray data);
    void ReceiveCmd(QByteArray data);
    void ReceiveVoice(QByteArray data);
    QByteArray GetToken();
    QByteArray GetTimeZone();

    void RequestVoiceData(QByteArray sessionId, quint32 start);
    void OpusSaveOggHeader();

signals:
    void sigDebugLog(QString);

private:
    QLowEnergyDescriptor  m_descriptor;
    QByteArray m_tmpArray;
    quint32 m_nVoicefileSize = 0;
    quint32 m_nVoiceByteoffset = 0;
    QFile *m_pFile;
    quint32 serial_number = 0x2;
    quint32 OGG_FILE_ID = 0x0;
};

#endif // BLUETOOTHCONNECT_H
