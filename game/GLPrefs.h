#ifndef GL_PREFS_H
#define GL_PREFS_H

namespace GL {

struct HighScore {
    char name[17];
    int score;
    int level;
};
    
struct PrefsInfo {
    HighScore highScores[10];
    char highName[17];
};

class Prefs {
public:
    bool load(PrefsInfo& thePrefs);
    void save(const PrefsInfo& thePrefs);
};

}

#endif
