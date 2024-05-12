#include "QLabelPlayVideo2.h"

#include <QImage>

QLabelPlayVideo2::QLabelPlayVideo2(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	FFmpeg *ffmpeg = new FFmpeg(this);
	connect(ffmpeg, &FFmpeg::newFrame, this, &QLabelPlayVideo2::updateFrame);
}

QLabelPlayVideo2::~QLabelPlayVideo2()
{}

void QLabelPlayVideo2::updateFrame(AVFrame *frame)
{
	QImage image(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
	ui.label->setPixmap(QPixmap::fromImage(image));

	static int flag = 0;
	flag++;
	if (flag == 10)
	{
		QImage image2(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
		ui.label_2->setPixmap(QPixmap::fromImage(image2));
	}
	if (flag == 100)
	{
		QImage image3(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
		ui.label_3->setPixmap(QPixmap::fromImage(image3));
	}
	if (flag == 200)
	{
		QImage image4(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
		ui.label_4->setPixmap(QPixmap::fromImage(image4));
	}

	repaint();
}
