#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <gl/gL.h>
#include "GLGame.h"
#include "resources.h"
#include "resource.h"
#include <string>
#include <mutex>
#include <condition_variable>

class AppController {
public:
    AppController();
    bool init(HINSTANCE hInstance);
    void run();
    void handleGameEvent(GL::Game::Event event);
    void handleHighScoreNameCallback(const char *name, int place);
    void onRender();
    void gameThread();
    LRESULT dlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
    void exit();
private:
    HWND win;
    HACCEL accelerators;
    GL::Game *game;

    HGLRC hRC;
    HDC hDC;

    static LRESULT CALLBACK AppController::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void onMenu(WORD cmd);
    void onKey(WPARAM key, bool down);
    void onMouseDown(UINT x, UINT y);
    void resetScores();
    std::wstring getMenuText(UINT item);
    bool setMenuText(UINT item, const std::wstring& text);
    std::string name_;
    int place_;
    HANDLE thread_;
    bool exit_;
    bool threadClosed_;
    std::mutex mutex_;
    std::condition_variable condition_variable_;
};

namespace {
void gameCallback(GL::Game::Event event, void *context)
{
    static_cast<AppController*>(context)->handleGameEvent(event);
}

void highScoreNameCallback(const char *name, int place, void *context) {
    static_cast<AppController*>(context)->handleHighScoreNameCallback(name, place);
}
}

AppController::AppController()
    : game(nullptr)
    , thread_(nullptr)
    , exit_(false)
    , threadClosed_(false)
{
}

bool AppController::init(HINSTANCE hInstance)
{
    // Register the window class
    WNDCLASSEX winClass;
    ZeroMemory(&winClass, sizeof(WNDCLASSEX));
    winClass.cbSize = sizeof(winClass);
    winClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    winClass.lpfnWndProc = AppController::WndProc;
    winClass.cbWndExtra = sizeof(LONG_PTR);
    winClass.hInstance = hInstance;
    winClass.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    winClass.lpszClassName = L"MainWindow";
    winClass.lpszMenuName = MAKEINTRESOURCEW(IDR_MAINMENU);
    if (RegisterClassExW(&winClass) == 0) {
        return false;
    }

    // Load the accelerators from the resource file so menu keyboard shortcuts work.
    accelerators = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(ID_MENU_ACCELERATORS));
    if (accelerators == NULL) {
        return false;
    }

    // Create the window centered
    int w = 640, h = 460;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    win = CreateWindowW(winClass.lpszClassName, GL_GAME_NAME_W, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, x, y, w, h, NULL, NULL, hInstance, this);
    if (win == NULL) {
        return false;
    }
    
    // Setup OpenGL
    hDC = GetDC(win);
    if (hDC == NULL) {
        return false;
    }
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0 || !SetPixelFormat(hDC, pixelFormat, &pfd)) {
        return false;
    }
    hRC = wglCreateContext(hDC);
    if (hRC == NULL) {
        return false;
    }

    // Readjust the window so the client size matches our desired size
    RECT rcClient, rcWindow;
    POINT ptDiff;
    (void)GetClientRect(win, &rcClient);
    (void)GetWindowRect(win, &rcWindow);
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    (void)MoveWindow(win, rcWindow.left, rcWindow.top, w + ptDiff.x, h + ptDiff.y, TRUE);

    // Show the window
    (void)ShowWindow(win, SW_SHOWNORMAL);
    (void)UpdateWindow(win);

    // Setup default menus
    handleGameEvent(GL::Game::EventEnded);

    // Rename About menu item
    std::wstring title = getMenuText(ID_MENU_ABOUT);
    const std::wstring token = L"%1";
    title.replace(title.find(token), token.size(), GL_GAME_NAME_W);
    if (!setMenuText(ID_MENU_ABOUT, title)) {
        return false;
    }

    return true;
}

std::wstring AppController::getMenuText(UINT item)
{
    MENUITEMINFOW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(MENUITEMINFOW);
    info.fMask = MIIM_STRING;
    if (!GetMenuItemInfoW(GetMenu(win), item, FALSE, &info)) {
        return {};
    }
    ++info.cch;
    std::wstring text;
    text.resize(info.cch - 1);
    info.dwTypeData = (LPWSTR)text.data();
    if (!GetMenuItemInfoW(GetMenu(win), item, FALSE, &info)) {
        return {};
    }
    return text;
}

bool AppController::setMenuText(UINT item, const std::wstring& text)
{
    MENUITEMINFOW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(MENUITEMINFOW);
    info.fMask = MIIM_STRING;
    info.dwTypeData = (LPWSTR)text.data();
    info.cch = (UINT)text.size();
    return SetMenuItemInfoW(GetMenu(win), item, FALSE, &info) == TRUE;
}

namespace {
DWORD WINAPI gameThreadMain(LPVOID param)
{
    AppController *app = static_cast<AppController*>(param);
    app->gameThread();
    return 0;
}

LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    AppController *app;
    if (Msg == WM_INITDIALOG) {
        app = (AppController*)lParam;
        SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
    } else {
        app = (AppController*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);
    }
    return app->dlgProc(hWndDlg, Msg, wParam, lParam);
}
}

void AppController::gameThread()
{
    (void)wglMakeCurrent(GetDC(win), hRC);

    game = new GL::Game(gameCallback, highScoreNameCallback, this, nullptr);

    RECT r;
    (void)GetClientRect(win, &r);
    game->renderer()->resize(r.right - r.left, r.bottom - r.top);

    while (!exit_) {
        onRender();
    }

    std::unique_lock<std::mutex> locker(mutex_);
    threadClosed_ = true;
    condition_variable_.notify_all();
}

void AppController::run()
{
    DWORD threadID;
    thread_ = CreateThread(nullptr, 0, gameThreadMain, this, 0, &threadID);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        // Check for keystrokes for the menus
        if (!TranslateAcceleratorW(win, accelerators, &msg)) {
            (void)TranslateMessage(&msg);
            (void)DispatchMessageW(&msg);
        }
    }
}

void AppController::handleGameEvent(GL::Game::Event event)
{
    switch (event) {
    case GL::Game::EventStarted:
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 0), ID_MENU_NEW_GAME, MF_DISABLED | MF_GRAYED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 0), ID_MENU_PAUSE, MF_ENABLED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 0), ID_MENU_END_GAME, MF_ENABLED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_HELP, MF_DISABLED | MF_GRAYED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_HIGH_SCORES, MF_DISABLED | MF_GRAYED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_RESET_SCORES, MF_DISABLED | MF_GRAYED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_ABOUT, MF_DISABLED | MF_GRAYED);
        break;
    case GL::Game::EventEnded:
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 0), ID_MENU_NEW_GAME, MF_ENABLED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 0), ID_MENU_PAUSE, MF_DISABLED | MF_GRAYED);
        setMenuText(ID_MENU_PAUSE, L"&Pause Game\tCtrl+P");
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 0), ID_MENU_END_GAME, MF_DISABLED | MF_GRAYED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_HELP, MF_ENABLED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_HIGH_SCORES, MF_ENABLED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_RESET_SCORES, MF_ENABLED);
        (void)EnableMenuItem(GetSubMenu(GetMenu(win), 1), ID_MENU_ABOUT, MF_ENABLED);
        break;
    }
}

void AppController::handleHighScoreNameCallback(const char *name, int place)
{
    (void)name; (void)place;
    name_ = name;
    place_ = place;
    DialogBoxParamW(nullptr, MAKEINTRESOURCE(IDD_DLGHIGHSCORE), win, reinterpret_cast<DLGPROC>(DlgProc), (LPARAM)this);
    game->processHighScoreName(name_.c_str(), place);
}

LRESULT AppController::dlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg) {
    case WM_INITDIALOG:
    {
        std::string text;
        text.resize(100);
        GetDlgItemTextA(hWndDlg, IDC_SCORE_STATIC, (LPSTR)text.data(), (int)text.size());
        const std::string token = "%1";
        text.replace(text.find(token), token.size(), std::to_string(place_));
        SetDlgItemTextA(hWndDlg, IDC_SCORE_STATIC, text.c_str());
        SetDlgItemTextA(hWndDlg, IDC_NAME_EDIT, name_.c_str());
        return TRUE;
    }

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            name_.resize(100);
            GetDlgItemTextA(hWndDlg, IDC_NAME_EDIT, (LPSTR)name_.c_str(), (int)name_.size());
            EndDialog(hWndDlg, 0);
            return TRUE;
        }
        break;
    }

    (void)lParam;
    return DefWindowProc(hWndDlg, Msg, wParam, lParam);
}

void AppController::onKey(WPARAM key, bool down)
{
    GL::Game::Key gameKey;
    switch (key) {
    case VK_SPACE: gameKey = GL::Game::KeySpacebar; break;
    case VK_DOWN: gameKey = GL::Game::KeyDownArrow; break;
    case VK_UP: gameKey = GL::Game::KeyUpArrow; break;
    case VK_LEFT: gameKey = GL::Game::KeyLeftArrow; break;
    case VK_RIGHT: gameKey = GL::Game::KeyRightArrow; break;
    case 'A': gameKey = GL::Game::KeyA; break;
    case 'S': gameKey = GL::Game::KeyS; break;
    case 'F': gameKey = GL::Game::KeyF; break;
    case VK_OEM_1: gameKey = GL::Game::KeyColon; break;
    case VK_OEM_7: gameKey = GL::Game::KeyQuote; break;
    case VK_PRIOR: gameKey = GL::Game::KeyPageUp; break;
    case VK_NEXT: gameKey = GL::Game::KeyPageDown; break;
    default:
	    return;
    }
    if (down) {
	    game->handleKeyDownEvent(gameKey);
    } else {
	    game->handleKeyUpEvent(gameKey);
    }
}

void AppController::onMouseDown(UINT x, UINT y)
{
    game->handleMouseDownEvent(GL::Point(x, y));
}

LRESULT CALLBACK AppController::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        AppController *appController = (AppController *)pcs->lpCreateParams;
        (void)SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(appController));
        result = 1;
    } else {
        AppController *appController = reinterpret_cast<AppController *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        bool wasHandled = false;

        if (appController != NULL) {
            switch (message) {
            case WM_DESTROY:
                appController->exit();
                PostQuitMessage(0);
                result = 1;
                wasHandled = true;
                break;

            case WM_COMMAND:
                appController->onMenu(LOWORD(wParam));
                result = 1;
                wasHandled = true;
                break;

            case WM_KEYDOWN:
            case WM_KEYUP:
	            appController->onKey(wParam, message == WM_KEYDOWN);
	            result = 0;
	            wasHandled = true;
	            break;

            case WM_LBUTTONDOWN:
                appController->onMouseDown(lParam & 0xFFFF, (lParam >> 16) & 0xFFFF);
                break;
            }
        }

        if (!wasHandled) {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

void AppController::exit()
{
    exit_ = true;
    std::unique_lock<std::mutex> locker(mutex_);
    condition_variable_.wait(locker, [this]{
        return threadClosed_;
    });
    CloseHandle(thread_);
}

void AppController::onRender()
{
    game->run();
    (void)SwapBuffers(hDC);
}

void AppController::onMenu(WORD cmd)
{
    switch(cmd) {
    case ID_MENU_NEW_GAME:
        game->newGame();
        break;
    case ID_MENU_PAUSE:
        game->pauseResumeGame();
        setMenuText(ID_MENU_PAUSE, game->paused() ? L"&Resume Game\tCtrl+R" : L"&Pause Game\tCtrl+P");
        break;
    case ID_MENU_EXIT:
        PostMessage(win, WM_CLOSE, 0, 0);
        break;
    case ID_MENU_END_GAME:
        game->endGame();
        break;
    case ID_MENU_HELP:
        game->showHelp();
        break;
    case ID_MENU_HIGH_SCORES:
        game->showHighScores();
        break;
    case ID_MENU_RESET_SCORES:
        resetScores();
        break;
    case ID_MENU_ABOUT:
        game->showAbout();
        break;
    }
}

void AppController::resetScores()
{
    const int result = MessageBoxW(win, L"Are you sure you want to reset " GL_GAME_NAME_W "'s scores?", L"Reset Scores", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
    if (result == IDYES) {
        game->resetHighScores();
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    AppController appController;
    if (!appController.init(hInstance)) {
        (void)MessageBoxW(NULL, L"Failed to initialize.", NULL, MB_OK | MB_ICONERROR);
        return 0;
    }
    appController.run();
    return 0;
}
