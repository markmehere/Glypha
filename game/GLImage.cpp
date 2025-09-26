#include "GLImage.h"

GL::Image::Image()
    : texture_(0)
    , width_(0)
    , height_(0)
    , alpha_(false)
    , colorBlending_(false)
{
}

GL::Image::~Image() {
    glDeleteTextures(1, &texture_);
}

bool GL::Image::isLoaded() const
{
    return texture_ != 0;
}

void GL::Image::unload() {
    glDeleteTextures(1, &texture_);
    texture_ = 0;
    width_ = 0;
    height_ = 0;
}

void GL::Image::loadTextureData_(const void *texData, bool hasAlpha)
{
    #if defined(EMSCRIPTEN) || defined(__ANDROID__)
    loadTextureData_(texData, hasAlpha ? GL_RGBA : GL_RGB, hasAlpha);
    #else
    loadTextureData_(texData, hasAlpha ? GL_BGRA_EXT : GL_BGR_EXT, hasAlpha);
    #endif
}

void GL::Image::loadTextureData_(const void *texData, GLenum format, bool hasAlpha)
{
    alpha_ = hasAlpha;
    
	// set pixel modes
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width_);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	// generate new texture name and bind it
    glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	
	// set texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, format, GL_UNSIGNED_BYTE, texData);
}

void GL::Image::drawBacking(const GL::Rect& destRect, float zoom) const
{
    if (!isLoaded() || zoom <= 0.0f) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_);

    if (alpha_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, colorBlending_ ? GL_BLEND : GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    float texWidth = width_ * zoom;
    float texHeight = height_ * zoom;

    float uRepeat = (destRect.right - destRect.left) / texWidth;
    float vRepeat = (destRect.bottom - destRect.top) / texHeight;

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(destRect.left,  destRect.top);
        glTexCoord2f(0.0f, vRepeat); glVertex2i(destRect.left,  destRect.bottom);
        glTexCoord2f(uRepeat, vRepeat); glVertex2i(destRect.right, destRect.bottom);
        glTexCoord2f(uRepeat, 0.0f); glVertex2i(destRect.right, destRect.top);
    glEnd();

    if (alpha_) {
        glDisable(GL_BLEND);
    }
    glDisable(GL_TEXTURE_2D);
}

void GL::Image::draw(const GL::Point *dest, size_t numDest, const GL::Point *src, size_t numSrc) const
{
    if (numDest != numSrc || numDest < 3) {
        // bug
        return;
    }
    
	// set this texture as current
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_);

    if (alpha_) {
        // enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    // GL_REPLACE prevents colors from seeping into a texture
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, colorBlending_ ? GL_BLEND : GL_REPLACE);

    // GL_NEAREST affects drawing the texture at different sizes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// draw the texture
	glBegin(numDest == 4 ? GL_QUADS : GL_POLYGON);
    for (size_t i = 0; i < numDest; ++i) {
        const GL::Point destPt = dest[i];
        const GL::Point srcPt = src[i];
        glTexCoord2f(static_cast<float>(srcPt.h) / static_cast<float>(width_), static_cast<float>(srcPt.v) / static_cast<float>(height_));
        glVertex2i(destPt.h, destPt.v);
    }
	glEnd();
	
    if (alpha_) {
        glDisable(GL_BLEND);
    }
	glDisable(GL_TEXTURE_2D);
}

void GL::Image::draw(const GL::Rect& destRect, const GL::Rect& srcRect) const
{
    GL::Point dest[4];
    GL::Point src[4];
    dest[0] = GL::Point(destRect.left, destRect.top);
    dest[1] = GL::Point(destRect.left, destRect.bottom);
    dest[2] = GL::Point(destRect.right, destRect.bottom);
    dest[3] = GL::Point(destRect.right, destRect.top);
    src[0] = GL::Point(srcRect.left, srcRect.top);
    src[1] = GL::Point(srcRect.left, srcRect.bottom);
    src[2] = GL::Point(srcRect.right, srcRect.bottom);
    src[3] = GL::Point(srcRect.right, srcRect.top);
    draw(dest, sizeof(dest) / sizeof(dest[0]), src, sizeof(src) / sizeof(src[0]));
}

void GL::Image::draw(const GL::Rect& destRect) const
{
    draw(destRect, GL::Rect(0, 0, width_, height_));
}

void GL::Image::draw(int x, int y) const
{
    draw(GL::Rect(x, y, width_, height_), GL::Rect(0, 0, width_, height_));
}

int GL::Image::width() const
{
    return width_;
}

int GL::Image::height() const
{
    return height_;
}

void GL::Image::setAllowColorBlending(bool colorBlending)
{
    colorBlending_ = colorBlending;
}
