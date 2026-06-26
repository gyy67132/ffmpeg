#pragma once

#include <QOpenGlWidget>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include "Thread.h"

extern "C"
{
#include "libavcodec/codec.h"
#include "libavcodec/packet.h"
#include "libavcodec/avcodec.h"   
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

class glWidgetYUV  : public QOpenGLWidget , protected QOpenGLFunctions_3_3_Core
{
	Q_OBJECT

public:
	glWidgetYUV(QWidget *parent = nullptr);
	~glWidgetYUV();

	void updateImage(AVFrame * image);
	void updateImage2(AVFrame* image);
protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();
private:
	QOpenGLTexture* textureY = nullptr;
	QOpenGLTexture* textureUV = nullptr;
	QOpenGLShaderProgram* shaderProgram = nullptr;
	QOpenGLVertexArrayObject* vao = nullptr;

	GLuint texY = 0;
	GLuint texU = 0;
	GLuint texV = 0;
};

