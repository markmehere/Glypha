#include "GLRenderer.h"
#include "GLResources.h"
#ifdef __ANDROID__
#include "AndroidOut.h"
#endif

int X_CORRECTION = 0;

void GL::Renderer::resize(int width, int height)
{
	GLsizei w = width, h = height;

	#if defined(MOBILE) || defined(__ANDROID__)
	w = (GL_GAME_WIDTH * h) / GL_GAME_HEIGHT;
	X_CORRECTION = (width - w) / 2;
	#endif

	glViewport(X_CORRECTION, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, GL_GAME_WIDTH, GL_GAME_HEIGHT, 0.0, 0.0, 1.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	bounds_.setSize(width, height);
}

void GL::Renderer::beginOutside()
{
    if (X_CORRECTION) {
        GLsizei w = bounds_.width(), h = bounds_.height();

        glViewport(0, 0, w, h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, w, h, 0.0, 0.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
}

void GL::Renderer::endOutside()
{
    if (X_CORRECTION) {
        GLsizei w, h = bounds_.height();

        w = (GL_GAME_WIDTH * h) / GL_GAME_HEIGHT;

        glViewport(X_CORRECTION, 0, w, h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, GL_GAME_WIDTH, GL_GAME_HEIGHT, 0.0, 0.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
}

void GL::Renderer::unload() {
    birdBackImg.unload();
}

void GL::Renderer::clear()
{
    if (didPrepare_ == false) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        didPrepare_ = true;
    }
    
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

    if (X_CORRECTION) {
        beginOutside();

        if (!birdBackImg.isLoaded()) {
            birdBackImg.load(birdback_png, birdback_png_len);
        }

        birdBackImg.drawBacking(bounds_, 1.5f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        setFillColor(0.0f, 0.0f, 0.0f); // black

        glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // 50% opacity
        fillRect(GL::Rect(X_CORRECTION - 12, 0, bounds_.width() - X_CORRECTION * 2 + 24, bounds_.height()));

        glColor4f(0.0f, 0.0f, 0.0f, 0.8f); // 50% opacity
        fillRect(GL::Rect(X_CORRECTION - 8, 0, bounds_.width() - X_CORRECTION * 2 + 16, bounds_.height()));
        glDisable(GL_BLEND);

        endOutside();
    }

}

void GL::Renderer::fillRect(const GL::Rect& rect)
{
	glBegin(GL_QUADS);
	glVertex2i(rect.left, rect.bottom);
	glVertex2i(rect.left, rect.top);
	glVertex2i(rect.right, rect.top);
	glVertex2i(rect.right, rect.bottom);
	glEnd();
}

void GL::Renderer::setFillColor(float red, float green, float blue)
{
    glColor3f(static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue));
}

GL::Rect GL::Renderer::bounds()
{
    return bounds_;
}

void GL::Renderer::beginLines(float lineWidth, bool smooth)
{
    if (smooth) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
}

void GL::Renderer::endLines()
{
	glEnd();
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void GL::Renderer::moveTo(int h, int v)
{
    lineStart_.h = h;
    lineStart_.v = v;
}

void GL::Renderer::lineTo(int h, int v)
{
	glVertex2i(lineStart_.h, lineStart_.v);
	glVertex2i(h, v);
}
