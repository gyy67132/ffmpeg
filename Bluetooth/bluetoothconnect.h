#ifndef BLUETOOTHCONNECT_H
#define BLUETOOTHCONNECT_H

#include <QObject>

#include <QLowEnergyController>

class BluetoothData;

class BluetoothConnect : public QObject
{
    Q_OBJECT
    QBluetoothDeviceInfo bluetoothDeviceInfo;
    QLowEnergyController *m_control;
    QList<QBluetoothUuid> m_uuidList;

    QLowEnergyService *m_service;

    QLowEnergyCharacteristic m_character;

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
signals:
    void sigDebugLog(QString);
};

#endif // BLUETOOTHCONNECT_H
