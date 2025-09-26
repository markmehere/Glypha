#include "GLMenu.h"

GL::Menu::Menu() :
    selected_(0),
    where_(0, 0)
{
}


GL::Menu::~Menu() {
}

void GL::Menu::draw(Point where, Renderer *r, const Font *f, const Image *fimg) {
    int zoom = 2;

    r->beginOutside();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // 40% opacity
    r->fillRect(Rect(where.h + 8 * zoom, where.v + 6 * zoom, 172 * zoom, 60 * zoom));
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // 40% opacity
    r->fillRect(Rect(where.h + 8 * zoom, where.v + 106 * zoom, 172 * zoom, 60 * zoom));
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // 40% opacity
    r->fillRect(Rect(where.h + 8 * zoom, where.v + 208 * zoom, 172 * zoom, 60 * zoom));
    glDisable(GL_BLEND);

    if (selected_ == 1) {
        r->setFillColor(0.537f, 0.341f, 0.145f);
        r->fillRect(Rect(where.h + 2 * zoom, where.v + 2 * zoom, 172 * zoom, 60 * zoom));
        r->setFillColor(0.086f, 0.098f, 0.152f);
        r->fillRect(Rect(where.h + 4 * zoom, where.v + 4 * zoom, 168 * zoom, 56 * zoom));
        r->setFillColor(0.710f, 0.451f, 0.184f);
        f->drawBigText("About", where.h + 20 * zoom, where.v + 21 * zoom, (float)zoom, *fimg);
    }
    else {
        r->setFillColor(0.710f, 0.451f, 0.184f);
        r->fillRect(Rect(where.h, where.v, 172 * zoom, 60 * zoom));
        r->setFillColor(0.118f, 0.129f, 0.188f);
        r->fillRect(Rect(where.h + 2 * zoom, where.v + 2 * zoom, 168 * zoom, 56 * zoom));
        r->setFillColor(0.710f, 0.451f, 0.184f);
        f->drawBigText("About", where.h + 20 * zoom, where.v + 21 * zoom, (float)zoom, *fimg);
    }

    if (selected_ == 2) {
        r->setFillColor(0.537f, 0.341f, 0.145f);
        r->fillRect(Rect(where.h + 2 * zoom, where.v + 102 * zoom, 172 * zoom, 60 * zoom));
        r->setFillColor(0.086f, 0.098f, 0.152f);
        r->fillRect(Rect(where.h + 4 * zoom, where.v + 104 * zoom, 168 * zoom, 56 * zoom));
        r->setFillColor(0.710f, 0.451f, 0.184f);
        f->drawBigText("New game", where.h + 20 * zoom, where.v + 120 * zoom, (float)zoom, *fimg);
    }
    else {
        r->setFillColor(0.710f, 0.451f, 0.184f);
        r->fillRect(Rect(where.h, where.v + 100 * zoom, 172 * zoom, 60 * zoom));
        r->setFillColor(0.118f, 0.129f, 0.188f);
        r->fillRect(Rect(where.h + 2 * zoom, where.v + 102 * zoom, 168 * zoom, 56 * zoom));
        r->setFillColor(0.710f, 0.451f, 0.184f);
        f->drawBigText("New game", where.h + 20 * zoom, where.v + 120 * zoom, (float) zoom, *fimg);
    }

    if (selected_ == 3) {
        r->setFillColor(0.537f, 0.341f, 0.145f);
        r->fillRect(Rect(where.h + 2 * zoom, where.v + 202 * zoom, 172 * zoom, 60 * zoom));
        r->setFillColor(0.086f, 0.098f, 0.152f);
        r->fillRect(Rect(where.h + 4 * zoom, where.v + 204 * zoom, 168 * zoom, 56 * zoom));
        r->setFillColor(0.710f, 0.451f, 0.184f);
        f->drawBigText("High scores", where.h + 20 * zoom, where.v + 219 * zoom, (float)zoom, *fimg);
    }
    else {
        r->setFillColor(0.710f, 0.451f, 0.184f);
        r->fillRect(Rect(where.h, where.v + 200 * zoom, 172 * zoom, 60 * zoom));
        r->setFillColor(0.118f, 0.129f, 0.188f);
        r->fillRect(Rect(where.h + 2 * zoom, where.v + 202 * zoom, 168 * zoom, 56 * zoom));
        r->setFillColor(0.710f, 0.451f, 0.184f);
        f->drawBigText("High scores", where.h + 20 * zoom, where.v + 219 * zoom, (float)zoom, *fimg);
    }

    r->endOutside();

    where_ = where;
}

void GL::Menu::tabDown() {
    selected_ += 1;
    if (selected_ > 3) selected_ = 1;
}

void GL::Menu::tabUp() {
    selected_ -= 1;
    if (selected_ < 1) selected_ = 3;
}

int GL::Menu::enter() {
    int oldSelected = selected_;
    if (selected_ == 2) selected_ = 0;
    return oldSelected;
}

int GL::Menu::handleMouseDownEvent(const Point& point) {
    int zoom = 2;
    selected_ = 0;
    if (point.h > where_.h && point.v > where_.v && point.h < where_.h + 172 * zoom && point.v < where_.v + 60 * zoom) {
        selected_ = 1;
    }
    if (point.h > where_.h && point.v > where_.v + 100 * zoom && point.h < where_.h + 172 * zoom && point.v < where_.v + 160 * zoom) {
        selected_ = 2;
    }
    if (point.h > where_.h && point.v > where_.v + 200 * zoom && point.h < where_.h + 172 * zoom && point.v < where_.v + 260 * zoom) {
        selected_ = 3;
    }
    return selected_;
}

int GL::Menu::handleMouseUpEvent(const Point& point) {
    int zoom = 2;
    if (selected_ == 1 && point.h > where_.h && point.v > where_.v && point.h < where_.h + 172 * zoom && point.v < where_.v + 60 * zoom) {
        selected_ = 0;
        return 1;
    }
    if (selected_ == 2 && point.h > where_.h && point.v > where_.v + 100 * zoom && point.h < where_.h + 172 * zoom && point.v < where_.v + 160 * zoom) {
        selected_ = 0;
        return 2;
    }
    if (selected_ == 3 && point.h > where_.h && point.v > where_.v + 200 * zoom && point.h < where_.h + 172 * zoom && point.v < where_.v + 260 * zoom) {
        selected_ = 0;
        return 3;
    }
    selected_ = 0;
    return 0;
}
