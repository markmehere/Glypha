//
// Compile for emscripten using
// emcc -Iinclude SingleFileOpenGLTex.cpp \
               -O2 -std=c++14 -s TOTAL_MEMORY=33554432 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file examples/data -s USE_SDL=2 -o html/SingleFileOpenGLTex.html
// where the following images must be located in a subfolder
//   - examples/data/test.png
//   - examples/data/cartman.png
//   - examples/data/cube-negx.png
//   - examples/data/cube-negz.png
//
// Tested against emscripten/1.37.14

#include <SDL2/SDL.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include "GLGlobals.h"
#include "GLGame.h"
#include "GLImage.h"
#include "GLResources.h"
#include "audio.h"

#include <iostream>
#include <fstream>
#include <vector>

extern int X_CORRECTION;

static bool quitting = false;
static SDL_Window *window = NULL;
static SDL_GLContext gl_context;

GL::Game *game;
GL::Image playerImg = GL::Image();

static bool display_size_changed = true;
static float xscale = 1.0;
static float yscale = 1.0;

static SDL_Keycode lastKey = 0;
static unsigned short lastButton = SDL_CONTROLLER_BUTTON_INVALID;
static short showSecondHelp = 0;
static short lastLeftRight = 0;

static int touches[5][3] = { 0 };

static void addTouch(int which, int id) {
    if (which == -1) return;
    for (int i = 0; i < 3; i++) {
        if (touches[which][i] == id) return;
        if (touches[which][i] != 0) {
            touches[which][i] = id;
            return;
        }
    }
    touches[which][0] = id;
}

static int removeTouch(int defaultWhich, int id) {
    int retValue = defaultWhich;
    for (int which = 0; which < 5; which++) {
        for (int i = 0; i < 3; i++) {
            if (touches[which][i] == id) {
                if (which != defaultWhich) SDL_Log("Finger %d has slid", id);
                touches[which][i] = 0;
                retValue = which;
            }
        }
    }
    if (retValue == -1) return -1;
    for (int i = 0; i < 3; i++) {
        touches[retValue][i] = 0;
    }
    return retValue;
}

static int getFingerId(long long value) {
    int result = (int)value;
    result += 16;
    return result;
}

static int getTouchRegion(int xpos, int ypos) {
    if (ypos < 305 && xpos < 320) {
        return 0;
    }
    else if (xpos < 320) {
        return 1;
    }
    else {
        return 2;
    }

    return -1;
}

SDL_GameController *findController() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return NULL;
}

SDL_GameController *controller = NULL;

static void update() {
    SDL_Event event;


    if (display_size_changed) {
        int w, h;
        emscripten_get_canvas_element_size("#canvas", &w, &h);
        SDL_Log("Resizing to %d %d", w, h);
        SDL_SetWindowSize(window, w, h);
        game->renderer()->resize(w, h);
        #ifdef MOBILE
        xscale = GL_GAME_WIDTH / (float)(w - X_CORRECTION * 2);
        #else
        xscale = GL_GAME_WIDTH / (float)w;
        #endif
        yscale = GL_GAME_HEIGHT / (float)h;
        display_size_changed = false;
    }


    while (SDL_PollEvent(&event)) {
        bool not_playing = !game->playing;
        bool hideAll = false;
        GL::Game::Key key = GL::Game::KeyNone;
        int touchRegion;
        int xpos;
        int ypos;
        short axis =  SDL_JoystickGetAxis(SDL_GameControllerGetJoystick(controller), 0);
        if (axis > 3500) {
            if (lastLeftRight < 0) game->handleKeyUpEvent(GL::Game::KeyLeftArrow);
            game->handleKeyDownEvent(GL::Game::KeyRightArrow);
            lastLeftRight = 1;
        }
        else if (axis < -3500) {
            if (lastLeftRight > 0) game->handleKeyUpEvent(GL::Game::KeyRightArrow);
            game->handleKeyDownEvent(GL::Game::KeyLeftArrow);
            lastLeftRight = -1;
        }
        else {
            if (lastLeftRight < 0) game->handleKeyUpEvent(GL::Game::KeyLeftArrow);
            if (lastLeftRight > 0) game->handleKeyUpEvent(GL::Game::KeyRightArrow);
            lastLeftRight = 0;
        }
        switch (event.type) {
            #ifndef MOBILE
            case SDL_MOUSEBUTTONDOWN:
                xpos = (int)((event.button.x - X_CORRECTION) * xscale);
                ypos = (int)(event.button.y * yscale);
                if (not_playing && ypos < 22 && xpos < 100) {
                    game->showAbout();
                }
                else if (not_playing && ypos < 22 && xpos < 200) {
                    game->showKeyboardWarn();
                }
                else if (not_playing && ypos < 22 && xpos < 300) {
                    game->showHelp();
                }
                else if (not_playing && ypos < 22 && xpos < 400) {
                    game->showHighScores();
                }
                else if (not_playing && ypos < 22 && xpos < 500) {
                    toggleAudio();
                }
                else {
                    game->handleMouseDownEvent(GL::Point(xpos, ypos));
                }
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if (!controller) {
                    controller = SDL_GameControllerOpen(event.cdevice.which);
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                if (controller && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
                    SDL_GameControllerClose(controller);
                    controller = findController();
                }
                break;
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
                if (controller && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
                    switch (event.cbutton.button) {
                        case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK:
                            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                                toggleAudio();
                            }
                            break;
                        case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START:
                            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                                game->pauseResumeGame();
                            }
                            break;
                        case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y:
                            if (not_playing) {
                                if (event.type == SDL_CONTROLLERBUTTONDOWN && lastButton == SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y) {
                                    game->hideAll(true);
                                }
                                else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                                    game->showHighScores();
                                }
                            }
                            break;
                        case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X:
                            if (not_playing) {
                                game->newGame();
                            }
                            break;
                        case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B:
                            if (not_playing) {
                                if (event.type == SDL_CONTROLLERBUTTONDOWN && lastButton == SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B) {
                                    if (showSecondHelp == 1) {
                                        game->handleKeyDownEvent(GL::Game::KeyPageDown);
                                        showSecondHelp = 2;
                                    }
                                    else {
                                        showSecondHelp = 0;
                                        game->hideAll(true);
                                    }
                                }
                                else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                                    game->showHelp();
                                    showSecondHelp = 1;
                                }
                            }
                            else if (game->paused() && event.type == SDL_CONTROLLERBUTTONDOWN) {
                                game->endGame();
                            }
                            break;
                        case SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A:
                            if (not_playing) {
                                if (event.type == SDL_CONTROLLERBUTTONDOWN && lastButton == SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A) {
                                    game->hideAll(true);
                                }
                                else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                                    game->showAbout();
                                }
                            }
                            else {
                                if (event.type == SDL_CONTROLLERBUTTONUP) game->handleKeyUpEvent(GL::Game::KeySpacebar);
                                else game->handleKeyDownEvent(GL::Game::KeySpacebar);
                            }
                            break;
                    }
                    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                        if (event.cbutton.button == lastButton && showSecondHelp == 0) lastButton = SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_INVALID;
                        else lastButton = event.cbutton.button;
                        if (event.cbutton.button != SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B) showSecondHelp = 0;
                    }
                }
                break;
            #else
                case SDL_FINGERUP:
                case SDL_FINGERDOWN:
                    if (!not_playing && !game->paused()) {
                        xpos = (int)(event.tfinger.x  * 640.0f);
                        ypos = (int)(event.tfinger.y * 480.0f);
                        touchRegion = getTouchRegion(xpos, ypos);
                        if (event.type == SDL_FINGERDOWN) {
                            addTouch(touchRegion, getFingerId(event.tfinger.fingerId));
                        }
                        else {
                            touchRegion = removeTouch(touchRegion, getFingerId(event.tfinger.fingerId));
                        }
                        switch (touchRegion) {
                            case 0:
                                if (event.type == SDL_FINGERDOWN) {
                                    game->handleKeyDownEvent(GL::Game::KeyLeftArrow);
                                }
                                else {
                                    game->handleKeyUpEvent(GL::Game::KeyLeftArrow);
                                }
                                break;
                            case 1:
                                if (event.type == SDL_FINGERDOWN) {
                                    game->handleKeyDownEvent(GL::Game::KeyRightArrow);
                                }
                                else {
                                    game->handleKeyUpEvent(GL::Game::KeyRightArrow);
                                }
                                break;
                            case 2:
                                if (event.type == SDL_FINGERDOWN) {
                                    game->handleKeyDownEvent(GL::Game::KeySpacebar);
                                }
                                else {
                                    game->handleKeyUpEvent(GL::Game::KeySpacebar);
                                }
                                break;
                        }
                    }
                    break;
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
                xpos = (int)((event.button.x - X_CORRECTION) * xscale);
                ypos = (int)(event.button.y * yscale);
                if (not_playing && event.type == SDL_MOUSEBUTTONUP && xpos >= 150 && ypos < 102 && xpos < 300) {
                    game->newGame();
                }
                if (not_playing && event.type == SDL_MOUSEBUTTONDOWN) {
                    if (xpos >= 0 && ypos < 102 && xpos < 150) {
                        game->showAbout();
                    }
                    else if (xpos >= 150 && ypos < 102 && xpos < 300) {
                        /* no action */
                    }
                    else if (xpos >= 300 && ypos < 102 && xpos < 450) {
                        game->showHighScores();
                    }
                    else {
                        game->handleMouseDownEvent(GL::Point(xpos, ypos));
                    }
                }
                else if (!not_playing && event.type == SDL_MOUSEBUTTONUP) {
                    if (xpos > 320 && xpos < 400 && ypos < 100) {
                        toggleAudio();
                        game->pauseResumeGame();
                    }
                    else if (game->paused() && ypos < 100 && xpos < 100) {
                        toggleAudio();
                        game->endGame();
                    }
                }
                break;
            #endif
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        if (event.type == SDL_KEYDOWN && game->paused()) {
                            game->endGame();
                        }
                        break;
                    case SDLK_m:
                        if (event.type == SDL_KEYDOWN) {
                            toggleAudio();
                        }
                        break;
                    case SDLK_p:
                    case SDLK_ESCAPE:
                        if (event.type == SDL_KEYDOWN) {
                            game->pauseResumeGame();
                        }
                        break;
                    case SDLK_SPACE:
                        key = GL::Game::KeySpacebar;
                        if (not_playing && lastKey == SDLK_h) {
                            key = GL::Game::KeyPageDown;
                        }
                        else {
                            game->hideAll(event.type == SDL_KEYDOWN);
                        }
                        break;
                    case SDLK_UP:
                        key = GL::Game::KeyUpArrow;
                        if (not_playing && event.type == SDL_KEYDOWN) {
                            //key = GL::Game::KeyPageUp;
                        }
                        break;
                    case SDLK_DOWN:
                        key = GL::Game::KeyDownArrow;
                        if (not_playing && event.type == SDL_KEYDOWN) {
                            //key = GL::Game::KeyPageDown;
                        }
                        break;
                    case SDLK_LEFT:
                        key = GL::Game::KeyLeftArrow;
                        break;
                    case SDLK_RIGHT:
                        key = GL::Game::KeyRightArrow;
                        break;
                    case SDLK_h:
                        game->hideAll(event.type == SDL_KEYDOWN);
                        if (not_playing && event.type == SDL_KEYDOWN && lastKey != SDLK_h) {
                            game->showHelp();
                        }
                        break;
                    case SDLK_n:
                        game->hideAll(event.type == SDL_KEYDOWN);
                        if (not_playing && event.type == SDL_KEYDOWN && event.type == SDL_KEYDOWN) {
                            game->newGame();
                        }
                        break;
                    case SDLK_a:
                        game->hideAll(event.type == SDL_KEYDOWN);
                        key = GL::Game::KeyA;
                        if (not_playing && event.type == SDL_KEYDOWN && lastKey != SDLK_a) {
                            game->showAbout();
                            key = GL::Game::KeyNone;
                        }
                        break;
                    case SDLK_s:
                        game->hideAll(event.type == SDL_KEYDOWN);
                        key = GL::Game::KeyS;
                        if (not_playing && event.type == SDL_KEYDOWN && lastKey != SDLK_s) {
                            game->showHighScores();
                            key = GL::Game::KeyNone;
                        }
                        break;
                    case SDLK_SEMICOLON:
                        key = GL::Game::KeyColon;
                        break;
                    case SDLK_QUOTEDBL:
                        key = GL::Game::KeyQuote;
                        break;
                    case SDLK_PAGEUP:
                        key = GL::Game::KeyPageUp;
                        break;
                    case SDLK_PAGEDOWN:
                        key = GL::Game::KeyPageDown;
                        break;
                    case SDLK_f:
                        game->hideAll(event.type == SDL_KEYDOWN);
                        key = GL::Game::KeyF;
                        break;
                    default:
                        game->hideAll(event.type == SDL_KEYDOWN);
                        break;
                }
                if (event.type == SDL_KEYUP) {
                    game->handleKeyUpEvent(key);
                } else {
                    if (lastKey == event.key.keysym.sym) lastKey = 0;
                    else lastKey = event.key.keysym.sym;
                    game->handleKeyDownEvent(key);
                }
                break;
        }
    }

    SDL_GL_MakeCurrent(window, gl_context);
    game->run();
    glFlush();
    SDL_GL_SwapWindow(window);
}


// string return, no parameters
EM_JS(char*, GetPlayerName, (), {
  let name;
  
  try {
    name = localStorage.getItem('glypha_hsname') || "Nemo";
  }
  catch (e) {
    console.error(e);
  }

  const byteCount = (Module.lengthBytesUTF8(name) + 1);
  
  const namePointer = Module._malloc(byteCount);
  Module.stringToUTF8(name, namePointer, byteCount);

  return namePointer;
});

static void saveScore(const char *ignore, int place, void *context) {
    SDL_Delay(100);
    char *namePointer = GetPlayerName();
    game->processHighScoreName(namePointer, place);
    free(namePointer);
    SDL_Delay(250);
    game->conclude();
}

static EM_BOOL on_web_display_size_changed(int event_type, const EmscriptenUiEvent *event, void *user_data )
{
    SDL_Log("Size changed");
    display_size_changed = true;  // custom global flag
    return 0;
}

int main(int argc, char *argv[]) {
    EM_ASM(
        FS.mkdir('/glypha');
        FS.mount(IDBFS, {}, '/glypha');
        FS.syncfs(true, function (err) {
            if (err) console.error("Cannot load IDBFS", err);
            else console.log("File system loaded");
        });
    );
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, false, on_web_display_size_changed);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    controller = findController();

    int w, h;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    SDL_Log("Canvas size: %d x %d", w, h);

    window = SDL_CreateWindow("Glypha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL);
    xscale = GL_GAME_WIDTH / (float)w;
    yscale = GL_GAME_HEIGHT / (float)h;
    initialize_gl4es();
    
    gl_context = SDL_GL_CreateContext(window);
    game = new GL::Game(NULL, saveScore, NULL, NULL);
    game->renderer()->resize(w, h);

    emscripten_set_main_loop(update, 60, 1);

    delete game;
    close_gl4es();
}
