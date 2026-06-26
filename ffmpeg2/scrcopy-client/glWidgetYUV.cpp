#include "glWidgetYUV.h"
#include <QThread>

glWidgetYUV::glWidgetYUV(QWidget *parent)
	: QOpenGLWidget(parent)
{}

glWidgetYUV::~glWidgetYUV()
{
}

void glWidgetYUV::initializeGL()
{
	this->initializeOpenGLFunctions();
	//Qt::HANDLE id = QThread::currentThreadId();
	
	glGenTextures(1, &texY);
	glBindTexture(GL_TEXTURE_2D, texY);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, IMAGIN_W, IMAGIN_H, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glGenTextures(1, &texU);
	glBindTexture(GL_TEXTURE_2D, texU);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, IMAGIN_W / 2, IMAGIN_H  / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glGenTextures(1, &texV);
	glBindTexture(GL_TEXTURE_2D, texV);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, IMAGIN_W / 2, IMAGIN_H / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	/*textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
	textureY->setSize(IMAGIN_W, IMAGIN_H);
	textureY->setFormat(QOpenGLTexture::R8_UNorm);
	textureY->allocateStorage();

	textureUV = new QOpenGLTexture(QOpenGLTexture::Target2D);
	textureUV->setSize(IMAGIN_W / 2, IMAGIN_H / 2);
	textureUV->setFormat(QOpenGLTexture::RG8_UNorm);
	textureUV->allocateStorage();*/

	const char* geometryShaderString = "#version 330 core\n" 
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec2 aTexCoord;\n"
		"out vec2 texCoord;"
		"void main(){\n"
		"	gl_Position = vec4(aPos, 1.0);\n"
		"	texCoord = aTexCoord;\n"
		"}";
	/*const char* fragmentShaderString = "#version 330 core\n"
		"in vec2 texCoord;\n"
		"uniform sampler2D texY;\n"
		"uniform sampler2D texUV;\n"
		"out vec4 fragmentColor;\n"
		"void main(){\n"
		"	float y = texture(texY, texCoord).r;\n"
		"	vec2 uv = texture(texUV, texCoord).rg;\n"
		"   float u = uv.r - 0.5;\n"
		"   float v = uv.g - 0.5;\n"
		"	float r = y + 1.402 * v;\n"
		"	float g = y - 0.344136 * u - 0.714136 * v;\n"
		"	float b = y + 1.772 * u;\n"
		"	fragmentColor = vec4(r, g, b, 1.0);\n"
		"}";*/
	const char* fragmentShaderString = "#version 330 core\n"
		"in vec2 texCoord;\n"
		"uniform sampler2D texY;\n"
		"uniform sampler2D texU;\n"
		"uniform sampler2D texV;\n"
		"out vec4 fragmentColor;\n"
		"void main(){\n"
		"	vec3 yuv;\n"
		"	vec3 rgb;\n"
		"	yuv.x = texture(texY, texCoord).r;\n"
		"	yuv.y = texture(texU, texCoord).r - 0.5;\n"
		"	yuv.z = texture(texV, texCoord).r - 0.5;\n"
		"	yuv.x = (yuv.x - 16.0/255.0) * (255.0 / (235.0 - 16.0));\n"
		"   rgb.r = yuv.x + 1.5748 * yuv.z;\n"
		"	rgb.g = yuv.x - 0.1873 * yuv.y - 0.4681 * yuv.z;\n"
		"	rgb.b = yuv.x + 1.8556 * yuv.y;\n"
		"	fragmentColor = vec4(rgb, 1.0);\n"
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
		-1.0, 1.0, 0.0,0.0, 0.0,
		-1.0, -1.0, 0.0,0.0, 1.0,
		1.0, -1.0, 0.0,1.0, 1.0,
		-1.0, 1.0, 0.0, 0.0, 0.0,
		1.0, -1.0, 0.0,1.0, 1.0,
		1.0, 1.0, 0.0,1.0, 0.0
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
void glWidgetYUV::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}
void glWidgetYUV::paintGL()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	shaderProgram->bind();
	/*shaderProgram->setUniformValue("texY", 0);
	shaderProgram->setUniformValue("texUV", 1);
	
	glActiveTexture(GL_TEXTURE0);
	textureY->bind();
	glActiveTexture(GL_TEXTURE1);
	textureUV->bind();*/

	shaderProgram->setUniformValue("texY", 0);
	shaderProgram->setUniformValue("texU", 1);
	shaderProgram->setUniformValue("texV", 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texY);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texU);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texV);

	vao->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vao->release();
}

void glWidgetYUV::updateImage(AVFrame* frame)
{
	makeCurrent();
	
	int w = frame->width;
	int h = frame->height;
	uint8_t* y_ptr = frame->data[0];
	uint8_t* u_ptr = frame->data[1];
	uint8_t* v_ptr = frame->data[2];

	textureY->destroy();
	textureY->setSize(w, h);
	textureY->setFormat(QOpenGLTexture::R8_UNorm);
	textureY->allocateStorage();
	textureY->setData(QImage(y_ptr, w, h, w, QImage::Format_Grayscale8).mirrored(false, true));

	textureUV->destroy();
	textureUV->setSize(w / 2, h / 2);
	textureUV->setFormat(QOpenGLTexture::RG8_UNorm);
	textureUV->allocateStorage();
	QImage uvImage(w / 2, h / 2, QImage::Format_RGBA8888);
	for (int i = 0; i < h / 2; i++)
		for (int j = 0; j < w / 2; j++)
			uvImage.setPixelColor(QPoint(j, i), QColor(u_ptr[i * (w/2) + j], v_ptr[i *(w/2) + j], 0 ,255));

	textureUV->setData(uvImage.mirrored(false, true));

	doneCurrent();
	update();
}

void glWidgetYUV::updateImage2(AVFrame* frame)
{
	makeCurrent();

	int w = frame->width;
	int h = frame->height;

	qDebug() << "Y[0]=" << (int)frame->data[0][0]
		<< " U[0]=" << (int)frame->data[1][0]
		<< " V[0]=" << (int)frame->data[2][0];

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glBindTexture(GL_TEXTURE_2D, texY);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[0]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, frame->data[0]);

	glBindTexture(GL_TEXTURE_2D, texU);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[1]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w/2, h/2, GL_RED, GL_UNSIGNED_BYTE, frame->data[1]);

	glBindTexture(GL_TEXTURE_2D, texV);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[2]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w / 2, h / 2, GL_RED, GL_UNSIGNED_BYTE, frame->data[2]);

	doneCurrent();
	update();
}