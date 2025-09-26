#include "GLRect.h"

#ifndef GLGAMESTATE_H
#define GLGAMESTATE_H

#define kNumLightningPts 8
#define kMaxEnemies 8

struct Player {
    GL::Rect dest, wasDest, wrap;
    int h, v;
    int wasH, wasV;
    int hVel, vVel;
    int srcNum, mode;
    int frame;
    bool facingRight, flapping;
    bool walking, wrapping;
    bool clutched;
};

struct Enemy {
    GL::Rect dest, wasDest;
    int h, v;
    int wasH, wasV;
    int hVel, vVel;
    int srcNum, mode;
    int kind, frame;
    int heightSmell, targetAlt;
    int flapImpulse, pass;
    int maxHVel, maxVVel;
    bool facingRight;
};

struct GameState {
    Player player;
    Enemy enemies[kMaxEnemies];
    int score;
    int numLedges;
    int levelOn;
    int livesLeft;
    int numEnemies;
    int numEnemiesThisLevel;
    int deadEnemies;
    int numOwls;
    int spawnedEnemies;
};

#endif // GLGAMESTATE_H
