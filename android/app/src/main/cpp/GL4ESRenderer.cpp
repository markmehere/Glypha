#include "GL4ESRenderer.h"
#include <memory>
#include <vector>
#include <android/imagedecoder.h>
#include "AndroidOut.h"
#include <cassert>
#include <ctime>

extern int X_CORRECTION;
extern std::string gPlayerName;
static GL::Game *gameInstance = nullptr;
static GL4ESRenderer *me = nullptr;
bool audioIsMuted = false;

void saveScore(const char *ignore, int place, void *context) {
    if (gameInstance) {
        const char *namePointer = gPlayerName.c_str();
        gameInstance->processHighScoreName(namePointer, place);
        gameInstance->conclude();
    }
}

void gameStartOrReset(GL::Game::Event event, void *context) {
    if (event == GL::Game::EventEnded) {
        me->resetTouches();
    }
}

int64_t getTickCountMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000LL + (int64_t)ts.tv_nsec / 1000000LL;
}

bool GL4ESRenderer::saveState() {
    if (game_ == nullptr) {
        return false;
    }

    if (app_->savedState != nullptr) {
        free(app_->savedState);
        app_->savedState = nullptr;
        app_->savedStateSize = 0;
    }

    if (!game_->playing) return true;

    app_->savedStateSize = sizeof(GameState);
    app_->savedState = malloc(app_->savedStateSize);

    if (app_->savedState == nullptr) {
        return false;
    }

    GameState state = game_->gameState();

    memcpy(app_->savedState, &state, app_->savedStateSize);

    return true;
}

void GL4ESRenderer::addTouch(int which, int id) {
    if (which == -1) return;
    for (int i = 0; i < 3; i++) {
        if (touches_[which][i] == id) return;
        if (touches_[which][i] != 0) {
            touches_[which][i] = id;
            return;
        }
    }
    touches_[which][0] = id;
}

int GL4ESRenderer::removeTouch(int defaultWhich, int id) {
    int retValue = defaultWhich;
    for (int which = 0; which < 5; which++) {
        for (int i = 0; i < 3; i++) {
            if (touches_[which][i] == id) {
                if (which != defaultWhich) aout << "Finger " << id << " has slid" << std::endl;
                touches_[which][i] = 0;
                retValue = which;
            }
        }
    }
    if (retValue == -1) return -1;
    for (int i = 0; i < 3; i++) {
        touches_[retValue][i] = 0;
    }
    return retValue;
}

void GL4ESRenderer::resetTouches() {
    for (auto& arr : touches_) {
        arr.fill(0);
    }
    upHeld_ = 0;
    downHeld_ = 0;
    leftHeld_ = false;
    rightHeld_ = false;
    lastUpHeld_ = 0;
    lastDownHeld_ = 0;
}

void GL4ESRenderer::initRenderer(GameState *gameState) {
    aout.precision(5);

    me = this;
    game_ = new GL::Game(gameStartOrReset, saveScore, nullptr, gameState);
    gameInstance = game_;
    wake();
}

GL4ESRenderer::~GL4ESRenderer() {
    sleep();
}

void GL4ESRenderer::sleep() {
    game_->sleep();
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
    close_gl4es();
}

void GL4ESRenderer::wake() {
    initialize_gl4es();

    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
    supportedConfigs.get(),
    supportedConfigs.get() + numConfigs,
    [&display](const EGLConfig &config) {
        EGLint red, green, blue, depth;
        if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
            && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
            && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
            && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

            aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                 << depth << std::endl;
            return red == 8 && green == 8 && blue == 8 && depth == 24;
        }
        return false;
    });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    // make width and height invalid so it gets updated the first frame in @a updateRenderArea()
    width_ = -1;
    height_ = -1;
}

int GL4ESRenderer::getTouchRegion(GL::Point point) {
    if (point.v < 305 && point.h < 320) {
        return 0;
    }
    else if (point.h < 320) {
        return 1;
    }
    else {
        return 2;
    }

    return -1;
}


void GL4ESRenderer::handleInput() {
    auto menu = game_->menu();
    static int32_t lastKey = AKEYCODE_UNKNOWN;

    // handle all queued inputs
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) {
        // no inputs yet.
        return;
    }

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);
        if ((motionEvent.source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK ||
            (motionEvent.source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD) {
            float xAxis = GameActivityPointerAxes_getAxisValue(&motionEvent.pointers[0], AMOTION_EVENT_AXIS_X);
            float yAxis = GameActivityPointerAxes_getAxisValue(&motionEvent.pointers[0], AMOTION_EVENT_AXIS_Y);
            const float DEAD_ZONE = 0.25f;

            if (xAxis < -DEAD_ZONE) {
                if (!leftHeld_) {
                    game_->handleKeyDownEvent(GL::Game::KeyLeftArrow);
                    leftHeld_ = true;
                }
            } else {
                if (leftHeld_) {
                    game_->handleKeyUpEvent(GL::Game::KeyLeftArrow);
                    leftHeld_ = false;
                }
            }

            if (xAxis > DEAD_ZONE) {
                if (!rightHeld_) {
                    game_->handleKeyDownEvent(GL::Game::KeyRightArrow);
                    rightHeld_ = true;
                }
            } else {
                if (rightHeld_) {
                    game_->handleKeyUpEvent(GL::Game::KeyRightArrow);
                    rightHeld_ = false;
                }
            }

            if (yAxis < -DEAD_ZONE) {
                if (!upHeld_) {
                    if (menu && getTickCountMs() > lastUpHeld_ + 40) {
                        lastKey = AKEYCODE_UNKNOWN;
                        menu->tabUp();
                    }
                    upHeld_ = getTickCountMs() + 200;
                }
            }
            else {
                game_->hideAll(true);
                if (upHeld_) lastUpHeld_ = upHeld_;
                upHeld_ = 0;
            }

            if (yAxis > DEAD_ZONE) {
                if (!downHeld_) {
                    if (menu && getTickCountMs() > lastDownHeld_ + 40) {
                        lastKey = AKEYCODE_UNKNOWN;
                        menu->tabDown();
                    }
                    downHeld_ = getTickCountMs() + 200;
                }
            }
            else {
                game_->hideAll(true);
                if (downHeld_) lastDownHeld_ = downHeld_;
                downHeld_ = 0;
            }
        }
        else {
            auto point = GL::Point((int)x, (int)y);
            auto correctedPoint = GL::Point((int)(((x - X_CORRECTION) / (float)(width_ - 2 * X_CORRECTION)) * GL_GAME_WIDTH), (int)((y / (float)height_) * GL_GAME_HEIGHT));
            auto isMouseDown = (action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_DOWN ||
                               (action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_POINTER_DOWN;

            switch (action & AMOTION_EVENT_ACTION_MASK) {
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                case AMOTION_EVENT_ACTION_CANCEL:
                case AMOTION_EVENT_ACTION_UP:
                case AMOTION_EVENT_ACTION_POINTER_UP:
                    if ((!game_->playing || game_->paused()) && correctedPoint.h > 500 && correctedPoint.h < 640 && correctedPoint.v < 60 && isMouseDown) {
                        audioIsMuted = !audioIsMuted;
                        if (!game_->playing) game_->handleMouseDownEvent(correctedPoint);
                    }
                    else if (game_->paused() && correctedPoint.h < 140 && correctedPoint.v < 60 && isMouseDown) {
                        game_->endGame();
                    }
                    else if (game_->playing) {
                        if (correctedPoint.h > 250 && correctedPoint.h < 390 && correctedPoint.v > 60 && correctedPoint.v < 160 && isMouseDown) {
                            game_->pauseResumeGame();
                        }
                        else {
                            auto touchRegion = getTouchRegion(correctedPoint);
                            if (isMouseDown) {
                                addTouch(touchRegion, (int)pointer.id + 1);
                            }
                            else {
                                touchRegion = removeTouch(touchRegion, (int)pointer.id + 1);
                            }
                            switch (touchRegion) {
                                case 0:
                                    if (isMouseDown) {
                                        game_->handleKeyDownEvent(GL::Game::KeyLeftArrow);
                                    }
                                    else {
                                        game_->handleKeyUpEvent(GL::Game::KeyLeftArrow);
                                    }
                                    break;
                                case 1:
                                    if (isMouseDown) {
                                        game_->handleKeyDownEvent(GL::Game::KeyRightArrow);
                                    }
                                    else {
                                        game_->handleKeyUpEvent(GL::Game::KeyRightArrow);
                                    }
                                    break;
                                case 2:
                                    if (isMouseDown) {
                                        game_->handleKeyDownEvent(GL::Game::KeySpacebar);
                                    }
                                    else {
                                        game_->handleKeyUpEvent(GL::Game::KeySpacebar);
                                    }
                                    break;
                            }
                        }
                    }
                    else if (game_->aboutVisible) {
                        game_->hideAll(true);
                    }
                    else if (game_->wallMode != GL::Game::WallMode::kWallModeNone) {
                        if (isMouseDown) {
                            if (game_->wallMode == GL::Game::WallMode::kWallModeHelp) {
                                game_->scrollHelpOrClose();
                            }
                            else {
                                game_->hideAll(true);
                            }
                        }
                    }
                    else {
                        if (menu) {
                            if (isMouseDown) {
                                auto choice = menu->handleMouseDownEvent(point);
                                // not a bug - the absolute point position is checked but the corrected point is sent
                                if (!choice && point.h > (int)((float)width_ * 0.05f) && point.h < (int)((float)width_ * 0.95f)) {
                                    if (correctedPoint.h > 250 && correctedPoint.h < 390 && correctedPoint.v > 60 && correctedPoint.v < 160) {
                                        game_->showHelp();
                                    }
                                    game_->handleMouseDownEvent(correctedPoint);
                                }
                            }
                            else {
                                auto choice = menu->handleMouseUpEvent(point);
                                switch (choice) {
                                    case 1:
                                        game_->showAbout();
                                        break;
                                    case 2:
                                        game_->newGame();
                                        resetTouches();
                                        break;
                                    case 3:
                                        game_->showHighScores();
                                        break;
                                }
                            }
                        }
                        else {
                            if (isMouseDown) {
                                game_->handleMouseDownEvent(correctedPoint);
                            }
                        }
                    }
                    break;
            }
        }
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);

    // handle input key events.
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        auto isKeyDown = keyEvent.action == AKEY_EVENT_ACTION_DOWN;
        auto not_playing = !game_->playing;
        auto key = GL::Game::KeyNone;
        switch (keyEvent.action) {
            case AKEY_EVENT_ACTION_DOWN:
            case AKEY_EVENT_ACTION_UP:
                switch (keyEvent.keyCode) {
                    case AKEYCODE_BUTTON_SELECT:
                    case AKEYCODE_Q:
                        if (isKeyDown && game_->playing && game_->paused()) {
                            game_->endGame();
                        }
                        break;
                    case AKEYCODE_BUTTON_Y:
                    case AKEYCODE_M:
                        if (isKeyDown) {
                            audioIsMuted = !audioIsMuted;
                        }
                        break;
                    case AKEYCODE_P:
                    case AKEYCODE_ESCAPE:
                    case AKEYCODE_BUTTON_START:
                        if (isKeyDown) {
                            game_->pauseResumeGame();
                        }
                        break;
                    case AKEYCODE_BUTTON_A:
                    case AKEYCODE_BUTTON_B:
                    case AKEYCODE_SPACE:
                        if (game_->playing) {
                            key = GL::Game::KeySpacebar;
                        }
                        if (keyEvent.action == AKEYCODE_SPACE && not_playing && lastKey == AKEYCODE_H) {
                            key = GL::Game::KeyPageDown;
                        } else {
                            game_->hideAll(isKeyDown);
                            if (menu && isKeyDown &&
                                lastKey != AKEYCODE_BUTTON_A && lastKey != AKEYCODE_BUTTON_B && lastKey != AKEYCODE_SPACE
                            ) {
                                auto choice = menu->enter();
                                switch (choice) {
                                    case 1:
                                        game_->showAbout();
                                        break;
                                    case 2:
                                        game_->newGame();
                                        resetTouches();
                                        break;
                                    case 3:
                                        game_->showHighScores();
                                        break;
                                }
                            }
                        }
                        break;
                    case AKEYCODE_TAB:
                        game_->hideAll(isKeyDown);
                        if (menu && isKeyDown) {
                            if (keyEvent.modifiers & AMETA_SHIFT_ON) {
                                menu->tabUp();
                            }
                            else {
                                menu->tabDown();
                            }
                        }
                        break;
                    case AKEYCODE_ENTER:
                        game_->hideAll(isKeyDown);
                        if (menu && isKeyDown && lastKey != AKEYCODE_ENTER) {
                            auto choice = menu->enter();
                            switch (choice) {
                                case 1:
                                    game_->showAbout();
                                    break;
                                case 2:
                                    game_->newGame();
                                    resetTouches();
                                    break;
                                case 3:
                                    game_->showHighScores();
                                    break;
                            }
                        }
                        break;
                    case AKEYCODE_DPAD_UP:
                        key = GL::Game::KeyUpArrow;
                        break;
                    case AKEYCODE_DPAD_DOWN:
                        key = GL::Game::KeyDownArrow;
                        break;
                    case AKEYCODE_DPAD_LEFT:
                        key = GL::Game::KeyLeftArrow;
                        break;
                    case AKEYCODE_DPAD_RIGHT:
                        key = GL::Game::KeyRightArrow;
                        break;
                    case AKEYCODE_N:
                        game_->hideAll(isKeyDown);
                        if (not_playing && isKeyDown) {
                            game_->newGame();
                        }
                        break;
                    case AKEYCODE_A:
                        game_->hideAll(isKeyDown);
                        key = GL::Game::KeyA;
                        if (not_playing && isKeyDown && lastKey != AKEYCODE_A) {
                            game_->showAbout();
                            key = GL::Game::KeyNone;
                        }
                        break;
                    case AKEYCODE_BUTTON_X:
                    case AKEYCODE_H:
                        if (isKeyDown && not_playing) {
                            if (game_->wallMode == GL::Game::kWallModeHelp) {
                                game_->scrollHelpOrClose();
                            }
                            else {
                                game_->hideAll(isKeyDown);
                                game_->showHelp();
                            }
                        }
                        key = GL::Game::KeyNone;
                        break;
                    case AKEYCODE_S:
                        game_->hideAll(isKeyDown);
                        key = GL::Game::KeyS;
                        if (not_playing && isKeyDown && lastKey != AKEYCODE_S) {
                            game_->showHighScores();
                            key = GL::Game::KeyNone;
                        }
                        break;
                    case AKEYCODE_SEMICOLON:
                        key = GL::Game::KeyColon;
                        break;
                    case AKEYCODE_APOSTROPHE:
                        key = GL::Game::KeyQuote;
                        break;
                    case AKEYCODE_PAGE_UP:
                        key = GL::Game::KeyPageUp;
                        break;
                    case AKEYCODE_PAGE_DOWN:
                        key = GL::Game::KeyPageDown;
                        break;
                    case AKEYCODE_F:
                        game_->hideAll(isKeyDown);
                        key = GL::Game::KeyF;
                        break;
                }
                if (!isKeyDown) {
                    game_->handleKeyUpEvent(key);
                } else {
                    if (lastKey == keyEvent.keyCode) lastKey = AKEYCODE_UNKNOWN;
                    else lastKey = keyEvent.keyCode;
                    game_->handleKeyDownEvent(key);
                }
                break;
        }
        aout << std::endl;
    }
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);
}

void GL4ESRenderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        game_->renderer()->resize(width, height);
    }
}

void GL4ESRenderer::render() {
    auto menu = game_->menu();

    if (display_ == EGL_NO_DISPLAY || context_ == EGL_NO_CONTEXT) return;

    updateRenderArea();

    if (upHeld_ && (getTickCountMs() > upHeld_ + 200)) {
        if (menu) {
            menu->tabUp();
        }
        upHeld_ = getTickCountMs();
    }

    if (downHeld_ && (getTickCountMs() > downHeld_ + 200)) {
        if (menu) {
            menu->tabDown();
        }
        downHeld_ = getTickCountMs();
    }

    game_->run();
    glFlush();

    if (!eglSwapBuffers(display_, surface_)) {
        aout << "Swap buffers failed";
    }
}

void GL4ESRenderer::pause() {
    if (game_->playing && !game_->paused()) {
        game_->pauseResumeGame();
    }
}
