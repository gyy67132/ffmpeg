#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PlaySocketVideo.h"

#include "TcpSocket.h"

class PlaySocketVideo : public QMainWindow
{
    Q_OBJECT

public:
    PlaySocketVideo(QWidget *parent = nullptr);
    ~PlaySocketVideo();

private slots:
    void updateFrame(AVFrame* frame);

private:
    Ui::PlaySocketVideoClass ui;
};
