#include "glWidget.h"
#include <QThread>

glWidget::glWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{}

glWidget::~glWidget()
{
}

void glWidget::initializeGL()
{
	this->initializeOpenGLFunctions();
	Qt::HANDLE id = QThread::currentThreadId();
	texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	/*texture->setSize(IMAGIN_W, IMAGIN_H);
	texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
	texture->allocateStorage();*/
	texture->setData(QImage(IMAGIN_W, IMAGIN_H, QImage::Format_RGBA8888));

	const char* geometryShaderString = "#version 330 core\n" 
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec2 aTexCoord;\n"
		"out vec2 texCoord;"
		"void main(){\n"
		"	gl_Position = vec4(aPos, 1.0);\n"
		"	texCoord = aTexCoord;\n"
		"}";
	const char* fragmentShaderString = "#version 330 core\n"
		"in vec2 texCoord;\n"
		"uniform sampler2D textureIn;\n"
		"out vec4 fragmentColor;\n"
		"void main(){\n"
		"	fragmentColor = texture(textureIn,texCoord);\n"
		"}";
	shaderProgram = new QOpenGLShaderProgram;
	QOpenGLShader* geometryShader = new QOpenGLShader(QOpenGLShader::Vertex);
	if (!geometryShader->compileSourceCode(geometryShaderString))
	{
		return;
	}
	if (!shaderProgram->addShader(geometryShader))
	{
		return;
	}
	QOpenGLShader* fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
	if (!fragmentShader->compileSourceCode(fragmentShaderString))
	{
		return;
	}
	if (!shaderProgram->addShader(fragmentShader))
	{
		return;
	}
	if (!shaderProgram->link())
	{
		qDebug() << Q_FUNC_INFO <<shaderProgram->log();
		return;
	}
	
	const float vertex[30] = {
		-1.0, 1.0, 0.0,0.0, 1.0,
		-1.0, -1.0, 0.0,0.0, 0.0,
		1.0, -1.0, 0.0,1.0, 0.0,
		-1.0, 1.0, 0.0, 0.0, 1.0,
		1.0, -1.0, 0.0,1.0, 0.0,
		1.0, 1.0, 0.0,1.0, 1.0
	};
	

	vao = new QOpenGLVertexArrayObject;
	vao->create();
	vao->bind();

	QOpenGLBuffer* vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vbo->create();
	vbo->bind();
	vbo->allocate(vertex, 30 * sizeof(float));
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3*sizeof(GL_FLOAT)));

	vao->release();
}
void glWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}
void glWidget::paintGL()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	shaderProgram->bind();
	shaderProgram->setUniformValue("textureIn", 0);
	glActiveTexture(GL_TEXTURE0);
	texture->bind();

	vao->bind();
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void glWidget::updateImage(QImage image)
{
	makeCurrent();
	QImage::Format a = image.format(); //QImage::Format_RGBA8888;
	//QImage converted = image.convertToFormat(QImage::Format_RGBA8888);
	qDebug() << "Image size:" << image.size()
		<< "Pixel(0,0):" << image.pixel(0, 0);
	qDebug() << "Texture size:" << IMAGIN_W << "x" << IMAGIN_H
		<< "QImage size:" << image.width() << "x" << image.height();
	texture->destroy();
	texture->setData(image.mirrored(false, true));
	doneCurrent();
	update();
}