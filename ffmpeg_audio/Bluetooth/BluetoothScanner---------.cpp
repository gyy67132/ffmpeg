﻿#include "bluetoothscanner.h"

#include "bluetoothconnect.h"

BluetoothScanner::BluetoothScanner(QObject *parent) :
    QObject(parent), discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    //超时时间为0，则一直扫描直到调用停止扫描，如果不为0，则扫描指定时间后自动停止
    discoveryAgent->setLowEnergyDiscoveryTimeout(0);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothScanner::deviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothScanner::scanFinished);
    connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Erro)),
            this, SLOT(scanError(QBluetoothDeviceDiscoveryAgent::Error)));
}

void BluetoothScanner::startScan()
{
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    emit bluetoothInfo("Scan start\n");
}

void BluetoothScanner::stopScan()
{
    discoveryAgent->stop();
}

void BluetoothScanner::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    emit bluetoothInfo("Device Discovered:" + device.name() + "(" + device.address().toString() + ")" + QString::number(device.deviceUuid().toUInt32()) + "\n");
    qDebug() << "Device Discovered:" << device.name() << "(" << device.address().toString() << ")";

    if(device.name() == "ToAll L-Ring" ){
        //new BluetoothConnect(device);
        //emit bluetoothInfo("------------------------start to connect ToAll L-Ring\n");
    }
    if(device.name() == "vivo X80"){
        //new BluetoothConnect(device);
        //emit bluetoothInfo("------------------------start to connect vivo X80\n");
    }
    // if(device.name() == "TOAll L_Ring Pro 2" && device.address().toString().contains("83")){
    //     new BluetoothConnect(device);
    //     emit bluetoothInfo("------------------------start to connect TOAll L_Ring Pro 2\n");
    // }
    if(device.address().toString().contains("DA:5E")){
        //正在扫描先停止
        stopScan();
        //连接指定设备
        new BluetoothConnect(device);
        emit bluetoothInfo("------------------------start to connect TOAll L_Ring Pro x\n");
    }
}

void BluetoothScanner::scanFinished()
{
    emit bluetoothInfo("Scan finished\n");
    qDebug() << "Scan finished";
}

void BluetoothScanner::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    emit bluetoothInfo("Scan error:" + QString::number(error) + "\n");
    qDebug() << "Scan error:" << error;
}
