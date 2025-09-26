#ifndef GLGAME_H
#define GLGAME_H

#include "GLPoint.h"
#include "GLRenderer.h"
#include "GLImage.h"
#include "GLSounds.h"
#include "GLCursor.h"
#include "GLUtils.h"
#include "GLFont.h"
#include "GLPrefs.h"
#include "GLMenu.h"
#include "GLGameLock.h"
#include "GLGameState.h"

namespace GL {

#define kNumLightningPts 8
#define kMaxEnemies 8

class Game {
public:
    enum Key {
        KeyNone        = (1 << 0),
        KeySpacebar    = (1 << 1),
        KeyUpArrow     = (1 << 2),
        KeyDownArrow   = (1 << 3),
        KeyLeftArrow   = (1 << 4),
        KeyRightArrow  = (1 << 5),
        KeyA           = (1 << 6),
        KeyS           = (1 << 7),
        KeyColon       = (1 << 8),
        KeyQuote       = (1 << 9),
        KeyPageUp      = (1 << 10),
        KeyPageDown    = (1 << 11),
        KeyF           = (1 << 12),
    };
    
    enum Event {
        EventStarted = 0,
        EventEnded = 1,
    };

    typedef void (*Callback)(Event event, void *context);
    typedef void (*HighScoreNameCallback)(const char *name, int place, void *context);

    Game(Callback callback, HighScoreNameCallback highScoreCallback, void *context, GameState *appState);
    ~Game();
    
    Renderer* renderer();
    
    void run();

    GameState gameState();
    void sleep();
    
    void handleMouseDownEvent(const Point& point);

    void handleKeyDownEvent(Key key);
    void handleKeyUpEvent(Key key);
    
    void newGame();
    void pauseResumeGame();
    bool paused() const;
    void endGame();
    void showHelp();
    void scrollHelpOrClose();
    void showHighScores();
    void resetHighScores();
    void showAbout();
    void hideAll(bool value);
    void processHighScoreName(const char *name, int place);
    
    void setShowFPS(bool show);
    bool showFPS() const;
    void conclude();
    void showKeyboardWarn();

    Menu *menu();

    bool playing;
    bool aboutVisible;
    enum WallMode {
        kWallModeNone = 0,
        kWallModeHelp = 1,
        kWallModeHighScores = 2,
    };
    WallMode wallMode;
    
private:
    Callback callback_;
    HighScoreNameCallback highScoreCallback_;
    void *callbackContext_;
    
    Renderer *renderer_;
    Menu *menu_;
    Cursor cursor;
    Sounds sounds;
    Utils utils;
    Lock lock_;
    
    double now;
    double lastTime;
    double accumulator;
    void loadImages();
    bool pausing, evenFrame, flapKeyDown;

    bool showFPS_;
    double fps_time;
    unsigned fps;
    char fps_buf[100];

    void update();
    void drawFrame() const;
    
    Image bgImg;
    void drawBackground() const;

    Image torchesImg;
    Rect flameDestRects[2], flameRects[4];
    void drawTorches() const;

    void handleLightning();
    void generateLightning(int h, int v);
    void drawLightning() const;
    void doLightning(const Point& point, int count);
    Point leftLightningPts[kNumLightningPts], rightLightningPts[kNumLightningPts];
    int lightningCount;
    double lastLightningStrike;
    Point lightningPoint;
    int newGameLightning;
    double lastNewGameLightning;
    Rect obeliskRects[4];
    Image obelisksImg;
    bool flashObelisks;
    void drawObelisks() const;
    int numLedges, levelOn, livesLeft;
    Player thePlayer;
    Rect playerRects[11];
    void resetPlayer(bool initialPlace);
    void offAMortal();
    Image playerImg;
    Image playerIdleImg;
    void drawPlayer() const;
    void movePlayer();
    void handlePlayerIdle();
    void handlePlayerWalking();
    void handlePlayerFlying();
    void handlePlayerSinking();
    void handlePlayerFalling();
    void handlePlayerBones();
    void setAndCheckPlayerDest();
    void checkTouchDownCollision();
    void checkPlatformCollision();
    void setUpLevel();
    void checkLavaRoofCollision();
    void checkPlayerWrapAround();
    void keepPlayerOnPlatform();
    
    void getPlayerInput();
    int keys_;
    Rect platformRects[6], touchDownRects[6], enemyRects[24];
    
    Rect platformCopyRects[9];
    void drawPlatforms() const;
    Image platformImg;
    int score;
    Image numbersImg;
    Rect numbersSrc[11], numbersDest[11];
    void drawLivesNumbers() const;
    void drawScoreNumbers() const;
    void drawLevelNumbers() const;
    void addToScore(int value);
    
    struct Hand {
        Rect dest;
        int mode;
    } theHand;
    Image handImg;
    Rect grabZone;
    Rect handRects[2];
    void initHandLocation();
    void handleHand();
    
    int countDownTimer;
    void handleCountDownTimer();
    
    int numEnemies;
    int numEnemiesThisLevel;
    int deadEnemies;
    int numOwls;
    int spawnedEnemies;
    Enemy theEnemies[kMaxEnemies];
    Rect enemyInitRects[5];
    Rect eggSrcRect;
    int doEnemyFlapSound;
	int doEnemyScrapeSound;
    Image enemyFly;
    Image enemyWalk;
    Image egg;
    void moveEnemies();
    void checkEnemyWrapAround(int who) const;
    void drawHand() const;
    void drawEnemies() const;
    void generateEnemies();
    bool setEnemyInitialLocation(Rect& theRect);
    void initEnemy(int i, bool reincarnated);
    void setEnemyAttributes(int i);
    int assignNewAltitude();
    void checkEnemyPlatformHit(int h);
    void checkEnemyRoofCollision(int i);
    void handleIdleEnemies(int i);
    void handleFlyingEnemies(int i);
    void handleWalkingEnemy(int i);
    void handleSpawningEnemy(int i);
    void handleFallingEnemy(int i);
    void handleEggEnemy(int i);
    void resolveEnemyPlayerHit(int i);
    void checkPlayerEnemyCollision();
    
    struct eyeInfo {
        Rect dest;
        int mode, opening;
        int srcNum, frame;
        bool killed, entering;
    } theEye;
    Image eyeImg;
    Rect eyeRects[4];
    void initEye();
    void killOffEye();
    void handleEye();
    void drawEye() const;
    
    Rect helpSrcRect;
    Rect helpSrc;
    Rect helpDest;
    Rect wallSrc;
    Rect wallDest;
    Rect warnFrame;
    Rect warnBody;
    bool showWarn;
    Image helpImg;
    enum WallState {
        kWallClosed = 0,
        kWallOpening = 1,
        kWallOpen = 2,
    };
    WallState wallState;
    int helpPos;
    
    void resetWall();
    void closeWall();
    void drawWall() const;
    
    void openHelp();
    void handleHelp();
    void drawHelp() const;
    void scrollHelp(int scrollDown);
    
    void openHighScores();
    void handleHighScores();
    void drawHighScores() const;
    void resetHighScores_();
    bool checkHighScore();

    GL::Font font11;
    Image font11Img;

    Rect scoreSrc;
    Rect scoreDest;
    const char *highScoresTitle;
    int highScoresTitleWidth;
    
    PrefsInfo thePrefs;
    
    void readInPrefs();

    Image aboutImg;
    void drawAbout(Renderer *r) const;
    void drawBanner(Renderer *r) const;

    Prefs prefs_;
};

}

#endif
