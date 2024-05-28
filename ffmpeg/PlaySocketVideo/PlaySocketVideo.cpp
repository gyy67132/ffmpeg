#include "PlaySocketVideo.h"


PlaySocketVideo::PlaySocketVideo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    TcpSocket* tcpSocket = new TcpSocket(this);
    

    connect(tcpSocket, &TcpSocket::getFrame, this, &PlaySocketVideo::updateFrame);
}

PlaySocketVideo::~PlaySocketVideo()
{}

void PlaySocketVideo::updateFrame(AVFrame* frame)
{
    QImage image(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
    ui.label->setPixmap(QPixmap::fromImage(image));

    repaint();
}