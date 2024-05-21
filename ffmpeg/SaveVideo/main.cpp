#include "SaveVideo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SaveVideo w;
    w.show();
    return a.exec();
}
