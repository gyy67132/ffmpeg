#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "globalmessage.h"
#include "ffmpegtool.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&GlobalMessage::instance(), &GlobalMessage::bluetoothInfo, this, &MainWindow::showBluetoothInfo2);

    mAudioCapture = new AudioCapture;

    connect(ui->pushButton, &QPushButton::clicked, this, [this](){
        mAudioCapture->startCaputer();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this](){
        mAudioCapture->stopCapture();
    });

    mAudioPlayer = new AudioPlayer;
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this](){
        mAudioPlayer->Play();
    });

    FFmpegTool *ffmpegTool = new FFmpegTool;
    connect(ui->pushButton_4, &QPushButton::clicked, this, [ffmpegTool](){
        ffmpegTool->start();
    });
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
