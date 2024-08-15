#ifndef GLOBALMESSAGE_H
#define GLOBALMESSAGE_H

#include <QObject>

class GlobalMessage : public QObject
{
    Q_OBJECT
public:
    static GlobalMessage& instance();

    void sendMessage(QString);
private:
    GlobalMessage(QObject *parent = nullptr);

signals:
    void bluetoothInfo(QString);

};

#endif // GLOBALMESSAGE_H
