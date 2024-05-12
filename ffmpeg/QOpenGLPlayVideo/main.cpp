#include "QOpenGLPlayVideo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QOpenGLPlayVideo w;
    w.show();
    return a.exec();
}
