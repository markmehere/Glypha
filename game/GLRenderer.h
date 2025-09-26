#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "GLRect.h"
#include "GLPoint.h"
#include "GLImage.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#elif defined(_WIN32)
#include <windows.h>
#include <gl/gl.h>
#elif defined(__HAIKU__)
#include <GL/gl.h>
#elif defined(GLYPHA_QT)
#include <QtOpenGL>
#endif

#define GL_GAME_WIDTH 640
#define GL_GAME_HEIGHT 460

namespace GL {

class Renderer {
public:
    void resize(int width, int height);
    void clear();
    
    void fillRect(const Rect& rect);
    void setFillColor(float red, float green, float blue);

    void beginLines(float lineWidth, bool smooth = true);
    void endLines();
    void moveTo(int h, int v);
    void lineTo(int h, int v);

    void beginOutside();
    void endOutside();
    
    Rect bounds();

    void unload();
    
private:
    Image birdBackImg;
    Rect bounds_;
    bool didPrepare_;
    Point lineStart_;
};

}

#endif
