#include "QOpenGLPlayVideo.h"

QOpenGLPlayVideo::QOpenGLPlayVideo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	FFmpeg *ffmpeg = new FFmpeg(this);
	connect(ffmpeg, &FFmpeg::newFrame, this, &QOpenGLPlayVideo::updateFrame);
}

QOpenGLPlayVideo::~QOpenGLPlayVideo()
{}

void QOpenGLPlayVideo::updateFrame(AVFrame *frame)
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
	if (flag == 50)
	{
		QImage image3(frame->data[0], frame->width, frame->height, QImage::Format_RGB888);
		ui.label_3->setPixmap(QPixmap::fromImage(image3));

		ui.openGLWidget->setImage(image3);
	}

	repaint();
}