#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PlayDemo.h"

class PlayDemo : public QMainWindow
{
    Q_OBJECT

public:
    PlayDemo(QWidget *parent = nullptr);
    ~PlayDemo();
protected:
    void timerEvent(QTimerEvent* e);

private:
    void cleanUp();

private:
    Ui::PlayDemoClass ui;
};

