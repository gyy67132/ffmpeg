#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QOpenGLPlayVideo.h"

#include "FFmpeg.h"

class QOpenGLPlayVideo : public QMainWindow
{
    Q_OBJECT

public:
    QOpenGLPlayVideo(QWidget *parent = nullptr);
    ~QOpenGLPlayVideo();

private slots:
	void updateFrame(AVFrame *frame);

private:
    Ui::QOpenGLPlayVideoClass ui;
};
