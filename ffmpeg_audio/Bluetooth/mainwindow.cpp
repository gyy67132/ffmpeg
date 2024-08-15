#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "globalmessage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&GlobalMessage::instance(), &GlobalMessage::bluetoothInfo, this, &MainWindow::showBluetoothInfo2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showBluetoothInfo(QString info)
{
    ui->textEdit->append(info);
}

void MainWindow::showBluetoothInfo2(QString info)
{
    ui->textEdit_2->append(info);
}
