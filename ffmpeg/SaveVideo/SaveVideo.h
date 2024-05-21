#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SaveVideo.h"

#include "FFmpeg.h"

class SaveVideo : public QMainWindow
{
    Q_OBJECT

public:
    SaveVideo(QWidget *parent = nullptr);
    ~SaveVideo();
private slots:
	void updateFrame(AVFrame *frame);
private:
    Ui::SaveVideoClass ui;
};
