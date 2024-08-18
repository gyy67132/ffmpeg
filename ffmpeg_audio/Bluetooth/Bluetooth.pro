QT       += core gui bluetooth concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/ffmpeg/include \
            $$PWD/SDL/include
LIBS += -L$$PWD/ffmpeg/bin/ -lavcodec -lavformat -lavutil -lswresample -lavdevice\
        -L$$PWD/SDL/lib/ -lSDL2

SOURCES += \
    AudioCapture.cpp \
    BluetoothScanner.cpp \
    audioplayer.cpp \
    bluetoothconnect.cpp \
    bluetoothdata.cpp \
    bufferutil.cpp \
    ffmpegtool.cpp \
    globalmessage.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    AudioCapture.h \
    BluetoothScanner.h \
    audioplayer.h \
    bluetoothconnect.h \
    bluetoothdata.h \
    bufferutil.h \
    ffmpegtool.h \
    globalmessage.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
