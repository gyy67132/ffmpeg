#include "GLPlayYUV.h"

GLPlayYUV::GLPlayYUV(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	FFmpeg *ffmpeg = new FFmpeg(this);
	connect(ffmpeg, &FFmpeg::newFrame, this, &GLPlayYUV::updateFrame);
}

GLPlayYUV::~GLPlayYUV()
{}
void GLPlayYUV::updateFrame(AVFrame *frame)
{
	QImage image(frame->data[0], frame->width, frame->height, QImage::Format_Indexed8);
	ui.label->setPixmap(QPixmap::fromImage(image));

	static int flag = 0;
	flag++;
	//if (flag == 10)
	{
		QImage image2(frame->data[1], frame->width/2, frame->height/2, QImage::Format_Indexed8);
		ui.label_2->setPixmap(QPixmap::fromImage(image2));
	}
	//if (flag == 50)
	{
		QImage image3(frame->data[2], frame->width/2, frame->height/2, QImage::Format_Indexed8);
		ui.label_3->setPixmap(QPixmap::fromImage(image3));
	}
	
	//FFmpeg::flip_frame_vertical(frame);
	ui.openGLWidget->updateYUV(frame);

	repaint();
}