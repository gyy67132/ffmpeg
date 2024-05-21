#pragma once

#include <QObject>

#include "FFmpeg.h"

class Recoder : QObject
{
	Q_OBJECT
public:
	Recoder();
	~Recoder();
	bool open();
	bool write(AVPacket *packet);
private:
	bool headerWritten = false;
};

