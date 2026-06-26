#include "TcpSocket.h"
#include <QtWidgets/QApplication>
#include "glWidget.h"
#include "glWidgetYUV.h"
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QDialog* dlg = new QDialog;
	dlg->setFixedSize(IMAGIN_W + 20, IMAGIN_H + 20);
	QHBoxLayout* layout = new QHBoxLayout(dlg);
	/*label = new QLabel;
	label->setScaledContents(false);
	label->setAlignment(Qt::AlignCenter);
	label->setFixedSize(IMAGIN_W, IMAGIN_H);
	layout->addWidget(label);*/

	int flagYUV = 1;
	if (!flagYUV) {
		glWidget* w = new glWidget;
		w->setFixedSize(IMAGIN_W, IMAGIN_H);
		layout->addWidget(w);

		TcpSocket* socket = new TcpSocket;

		QObject::connect(socket, &TcpSocket::signal_updateImage, w, [=](const QImage& img) {
			w->updateImage(img);
			}, Qt::QueuedConnection);
	}
	else {
		glWidgetYUV* w = new glWidgetYUV;
		w->setFixedSize(IMAGIN_W, IMAGIN_H);
		layout->addWidget(w);
		
		TcpSocket *socket = new TcpSocket;

		QObject::connect(socket, &TcpSocket::signal_updateFrame, w, [=](AVFrame *frame) {
			w->updateImage2(frame);
			}, Qt::QueuedConnection);
	}
	dlg->show();


    return app.exec();
}
