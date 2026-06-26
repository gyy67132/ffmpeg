#include <QtWidgets/QApplication>

#include "StreamController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    StreamController sc;
    return app.exec();
}
