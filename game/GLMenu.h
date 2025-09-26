#ifndef GLMENU_H
#define GLMENU_H

#include "GLPoint.h"
#include "GLRenderer.h"
#include "GLFont.h"

namespace GL {
class Menu {

public:
    Menu();
    ~Menu();

    void draw(Point where, Renderer *r, const Font *f, const Image *fimg);

    void tabDown();
    void tabUp();
    int enter();
    int handleMouseDownEvent(const Point& point);
    int handleMouseUpEvent(const Point& point);

private:

    int selected_;
    Point where_;

};
}

#endif
