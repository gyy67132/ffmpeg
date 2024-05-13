#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GLPlayYUV.h"

#include "FFmpeg.h"

class GLPlayYUV : public QMainWindow
{
    Q_OBJECT

public:
    GLPlayYUV(QWidget *parent = nullptr);
    ~GLPlayYUV();
private slots:
	void updateFrame(AVFrame *frame);
private:
    Ui::GLPlayYUVClass ui;
};
