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

//const QString fShader = R"(
//	varying vec2 textureOut;
//	uniform sampler2D textureY;
//	uniform sampler2D textureU;
//	uniform sampler2D textureV;
//	void main(){
//		vec3 yuv;
//		vec3 rgb;
//		yuv.x = texture2D(textureY, textureOut).r;
//		yuv.y = texture2D(textureU, textureOut).r - 0.5;
//		yuv.z = texture2D(textureV, textureOut).r - 0.5;
//		rgb = mat3(1,1,1,  0,-0.39465,2.03211,  1.13983, -0.58060,  0) * yuv;
//		gl_FragColor = vec4(rgb, 1.0);
//	}
//)";

const QString fShader = R"(
	varying vec2 textureOut;
	uniform sampler2D textureY;
	uniform sampler2D textureU;
	uniform sampler2D textureV;
	void main(){
		vec3 yuv;
		vec3 rgb;
		const vec3 Rcoeff = vec3(1.1644,  0.000,  1.7927);
        const vec3 Gcoeff = vec3(1.1644, -0.2132, -0.5329);
        const vec3 Bcoeff = vec3(1.1644,  2.1124,  0.000);

        // 根据指定的纹理textureY和坐标textureOut来采样
        yuv.x = texture2D(textureY, textureOut).r;
        yuv.y = texture2D(textureU, textureOut).r - 0.5;
        yuv.z = texture2D(textureV, textureOut).r - 0.5;

        // 采样完转为rgb
        // 减少一些亮度
        yuv.x = yuv.x - 0.0625;
        rgb.r = dot(yuv, Rcoeff);
        rgb.g = dot(yuv, Gcoeff);
        rgb.b = dot(yuv, Bcoeff);
        // 输出颜色值
        gl_FragColor = vec4(rgb, 1.0);
	}
)";

const GLfloat coordinate[] = {
	-1,-1,0.0,
	1.0,-1.0,0.0,
	-1.0,1.0,0.0,
	1.0,1.0,0.0,

	0.0,1.0,
	1.0,1.0,
	0.0,0.0,
	1.0,0.0
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

	glGenTextures(3, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
}
void OpenGLWidget::paintGL()
{
	if (!avFrame)
		return;
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, avFrame->width, avFrame->height, 0, GL_RED, GL_UNSIGNED_BYTE, avFrame->data[0]);
	shaderProgram.setUniformValue("textureY", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, avFrame->width/2, avFrame->height/2, 0, GL_RED, GL_UNSIGNED_BYTE, avFrame->data[1]);
	shaderProgram.setUniformValue("textureU", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, avFrame->width/2, avFrame->height/2, 0, GL_RED, GL_UNSIGNED_BYTE, avFrame->data[2]);
	shaderProgram.setUniformValue("textureV", 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void OpenGLWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}
void OpenGLWidget::updateYUV(AVFrame* frame)
{
	//if (avFrame)
		//av_frame_unref(avFrame);
	avFrame = frame;
	update();
}