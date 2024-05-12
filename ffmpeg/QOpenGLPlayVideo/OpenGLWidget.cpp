#include "OpenGLWidget.h"

const QString vShader = R"(
	attribute vec3 vertexIn;
	attribute vec2 textureIn;
	varying vec2 textureOut; 
	void main() 
	{
		gl_Position = vec4(vertexIn, 1.0);
		textureOut = textureIn;
	}
)";

const QString fShader = R"(
	varying vec2 textureOut;
	uniform sampler2D texture;
	void main(){
		gl_FragColor = texture2D(texture, textureOut);
	}
)";

const GLfloat coordinate[] = {
	-1,-1,0.0,
	1.0,-1.0,0.0,
	-1.0,1.0,0.0,
	1.0,1.0,0.0,

	0.0,0.0,
	1.0,0.0,
	0.0,1.0,
	1.0,1.0	
};


OpenGLWidget::OpenGLWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{}

OpenGLWidget::~OpenGLWidget()
{}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();

	vbo.create();
	vbo.bind();
	vbo.allocate(coordinate, sizeof(coordinate));

	shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vShader);
	shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fShader);
	shaderProgram.link();
	shaderProgram.bind();

	shaderProgram.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 3, 3 * sizeof(GL_FLOAT));
	shaderProgram.enableAttributeArray("vertexIn");
	shaderProgram.setAttributeBuffer("textureIn", GL_FLOAT, 12 * sizeof(GL_FLOAT), 2, 2*sizeof(GL_FLOAT));
	shaderProgram.enableAttributeArray("textureIn");

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	QImage image("./brick1upArrow.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//***************jpgÐèÒªRGBA
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
}
void OpenGLWidget::paintGL()
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void OpenGLWidget::resizeGL(int w, int h)
{
	
}
void OpenGLWidget::setImage(QImage image)
{
	makeCurrent();
	glBindTexture(GL_TEXTURE_2D, texture);
	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
	doneCurrent();
}
