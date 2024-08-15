#ifndef BLUETOOTHSCANNER_H
#define BLUETOOTHSCANNER_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QList>
#include <QObject>
#include <QDebug>

class BluetoothScanner : public QObject
{
    Q_OBJECT
public:
    BluetoothScanner(QObject *parent = nullptr);

    void startScan();

    void stopScan();

signals:
    void bluetoothInfo(QString);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

    void scanFinished();

    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;

};

#endif // BLUETOOTHSCANNER_H
