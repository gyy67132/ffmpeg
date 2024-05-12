#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QLabelPlayVideo2.h"

#include "FFmpeg.h"


class QLabelPlayVideo2 : public QMainWindow
{
    Q_OBJECT

public:
    QLabelPlayVideo2(QWidget *parent = nullptr);
    ~QLabelPlayVideo2();
private slots:
	void updateFrame(AVFrame *frame);

private:
    Ui::QLabelPlayVideo2Class ui;
};
