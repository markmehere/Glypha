#include <jni.h>

#include "AndroidOut.h"

#include "game/GLGame.h"
#include "GL4ESRenderer.h"

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

struct android_app *gAndroidApp;
bool gTouchEnabled = false;
std::string gPlayerName = "Player";

extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>
JNIEXPORT void JNICALL Java_org_pazolli_glypha_GlyphaActivity_nativeInitGameSettings(
        JNIEnv *env,
        jobject /* this */,
        jstring playerNameJStr,
        jboolean touchEnabledJBool
) {
    const char *playerNameNative = env->GetStringUTFChars(playerNameJStr, nullptr);
    bool touchEnabledNative = static_cast<bool>(touchEnabledJBool);

    aout << "Native received Player Name: " << playerNameNative << std::endl;
    aout << "Native received Touch Enabled: " << (touchEnabledNative ? "true" : "false") << std::endl;

    gTouchEnabled = touchEnabledNative;
    gPlayerName = std::string(playerNameNative);

    env->ReleaseStringUTFChars(playerNameJStr, playerNameNative);
}

void handle_cmd(android_app *pApp, int32_t cmd) {
    auto *pRenderer = reinterpret_cast<GL4ESRenderer *>(pApp->userData);
    static GameState *gameState = nullptr;
    switch (cmd) {
        case APP_CMD_RESUME:
            if (pApp->savedStateSize > 0) {
                gameState = (GameState *)malloc(sizeof(GameState));
                memcpy(gameState, pApp->savedState, sizeof(GameState));
            }
            break;
        case APP_CMD_INIT_WINDOW:
            if (pRenderer == nullptr) {
                pApp->userData = new GL4ESRenderer(pApp, gameState);
                if (gameState) {
                    free(gameState);
                    gameState = nullptr;
                }
            }
            else {
                pRenderer->wake();
            }
            break;
        case APP_CMD_SAVE_STATE:
            pRenderer->saveState();
            break;
        case APP_CMD_TERM_WINDOW:
            pRenderer->sleep();
            break;
        case APP_CMD_LOST_FOCUS:
            if (pRenderer) {
                pRenderer->pause();
            }
            break;
        case APP_CMD_DESTROY:
            if (pRenderer) {
                pApp->userData = nullptr;
                delete pRenderer;
            }
            gAndroidApp = nullptr;
            break;
        default:
            break;
    }
}

/*!
 * Enable the motion events you want to handle; not handled events are
 * passed back to OS for further processing. For this example case,
 * only pointer and joystick devices are enabled.
 *
 * @param motionEvent the newly arrived GameActivityMotionEvent.
 * @return true if the event is from a pointer or joystick device,
 *         false for all other input devices.
 */
bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
    auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
    return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
            sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *pApp) {
    // Can be removed, useful to ensure your code is running
    aout << "Welcome to Glypha III" << std::endl;
    gAndroidApp = pApp;

    // Register an event handler for Android events
    pApp->onAppCmd = handle_cmd;

    // Set input event filters (set it to NULL if the app wants to process all inputs).
    // Note that for key inputs, this example uses the default default_key_filter()
    // implemented in android_native_app_glue.c.
    android_app_set_motion_event_filter(pApp, motion_event_filter_func);

    // This sets up a typical game/event loop. It will run until the app is destroyed.
    do {
        // Process all pending events before running game logic.
        bool done = false;
        while (!done) {
            // 0 is non-blocking.
            int timeout = 0;
            int events;
            android_poll_source *pSource;
            int result = ALooper_pollOnce(timeout, nullptr, &events,
                                          reinterpret_cast<void**>(&pSource));
            switch (result) {
                case ALOOPER_POLL_TIMEOUT:
                    [[clang::fallthrough]];
                case ALOOPER_POLL_WAKE:
                    // No events occurred before the timeout or explicit wake. Stop checking for events.
                    done = true;
                    break;
                case ALOOPER_EVENT_ERROR:
                    aout << "ALooper_pollOnce returned an error" << std::endl;
                    break;
                case ALOOPER_POLL_CALLBACK:
                    break;
                default:
                    if (pSource) {
                        pSource->process(pApp, pSource);
                    }
            }
        }

        // Check if any user data is associated. This is assigned in handle_cmd
        if (pApp->userData) {
            auto *pRenderer = reinterpret_cast<GL4ESRenderer *>(pApp->userData);
            pRenderer->handleInput();
            pRenderer->render();
        }
    } while (!pApp->destroyRequested);
}

}