#ifndef GLIMAGE_H
#define GLIMAGE_H

#include "GLRect.h"
#include "GLPoint.h"
#include <stddef.h>

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

namespace GL {

class Image {
public:
    Image();
    ~Image();

    void load(const unsigned char *buf, size_t bufSize);
    void unload();
    bool isLoaded() const;
    
    void draw(const Point *dest, size_t numDest, const Point *src, size_t numSrc) const;
    void draw(const Rect& destRect, const Rect& srcRect) const;
    void draw(const Rect& destRect) const;
    void draw(int x, int y) const;
    void drawBacking(const Rect& destRect, float zoom = 1.0f) const;
    
    int width() const;
    int height() const;
    
    void setAllowColorBlending(bool colorBlending);

private:
	void loadTextureData_(const void *texData, bool hasAlpha = true);
	void loadTextureData_(const void *texData, GLenum format, bool hasAlpha = true);
    
    GLuint texture_;
    int width_, height_;
    bool alpha_;
    bool colorBlending_;
};
    
}

#endif
