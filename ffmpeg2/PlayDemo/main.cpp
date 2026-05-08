#include "PlayDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PlayDemo window;
    window.show();
    return app.exec();
}
