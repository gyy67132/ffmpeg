#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "FFmpeg.h"

class OpenGLWidget  : public QOpenGLWidget , public QOpenGLFunctions
{
	Q_OBJECT

public:
	OpenGLWidget(QWidget *parent);
	~OpenGLWidget();

	void setImage(QImage);
	void setImage(AVFrame *image);
protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;

private:
	QOpenGLBuffer vbo;
	QOpenGLShaderProgram shaderProgram;
	GLuint texture;
	//QOpenGLTexture texture;
};
