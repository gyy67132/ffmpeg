#pragma once

#include <QOpenGlWidget>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include "Thread.h"

class glWidget  : public QOpenGLWidget , protected QOpenGLFunctions_3_3_Core
{
	Q_OBJECT

public:
	glWidget(QWidget *parent = nullptr);
	~glWidget();

	void updateImage(QImage image);
protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();
private:
	QOpenGLTexture* texture = nullptr;
	QOpenGLShaderProgram* shaderProgram = nullptr;
	QOpenGLVertexArrayObject* vao = nullptr;
};

