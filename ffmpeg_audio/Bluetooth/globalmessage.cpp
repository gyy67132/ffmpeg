#include "globalmessage.h"




GlobalMessage::GlobalMessage(QObject *parent)
    : QObject{parent}
{

}

void GlobalMessage::sendMessage(QString value)
{
    emit bluetoothInfo(value);
}

GlobalMessage& GlobalMessage::instance()
{
    static GlobalMessage g_GlobalMessage;
    return g_GlobalMessage;
}
