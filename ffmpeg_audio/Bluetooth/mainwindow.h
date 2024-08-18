#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AudioCapture.h"
#include "audioplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showBluetoothInfo(QString info);
    void showBluetoothInfo2(QString info);

private:
    Ui::MainWindow *ui;

    AudioCapture *mAudioCapture;
    AudioPlayer *mAudioPlayer;
};
#endif // MAINWINDOW_H
