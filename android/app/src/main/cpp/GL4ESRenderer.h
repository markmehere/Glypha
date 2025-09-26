#ifndef GLYPHA_III_GL4ESRENDERER_H
#define GLYPHA_III_GL4ESRENDERER_H

#include <EGL/egl.h>
#include <memory>
#include "gl4es/include/GL/gl.h"
#include "gl4es/include/gl4esinit.h"
#include "game/GLGame.h"
#include "game/GLPoint.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>

struct android_app;

class GL4ESRenderer {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     */
    inline GL4ESRenderer(android_app *pApp, GameState *gameState) :
        app_(pApp),
        display_(EGL_NO_DISPLAY),
        surface_(EGL_NO_SURFACE),
        context_(EGL_NO_CONTEXT),
        width_(0),
        height_(0) {
            initRenderer(gameState);
        }

    virtual ~GL4ESRenderer();

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Renders all the models in the renderer
     */
    void render();

    /*!
     * Places the renderer to sleep
     */
    void sleep();

    /*!
     * Wakes up the renderer
     */
    void wake();

    /*!
     * Pause the game (used when the game loses focus)
     */
    void pause();

    /*!
     * Saves the game state, which happens as part of the Adnroid lifecycle
     */
    bool saveState();

    /*!
     * Resets all touches to off
     */
    void resetTouches();

private:
    /*!
     * Performs necessary OpenGL initialization. Customize this if you want to change your EGL
     * context or application-wide settings.
     */
    void initRenderer(GameState *gameState);

    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();
    
    /*!
     * Adds touch
     */
    void addTouch(int which, int id);

    /*!
     * Removes a touch
     */
    int removeTouch(int defaultWhich, int id);

    /*!
     * Determines which region of the screen was touched
     */
    int getTouchRegion(GL::Point point);

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;
    GL::Game *game_;
    int64_t upHeld_ = 0;
    int64_t downHeld_ = 0;
    bool leftHeld_ = false;
    bool rightHeld_ = false;
    int64_t lastUpHeld_ = 0;
    int64_t lastDownHeld_ = 0;
    std::array<std::array<int, 3>, 5> touches_{};
};


#endif //GLYPHA_III_GL4ESRENDERER_H
