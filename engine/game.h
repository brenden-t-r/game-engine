#ifndef GAMEENGINE_GAME_H
#define GAMEENGINE_GAME_H

#include "../platform/platform.h"

class Game {
public:
    explicit Game(Platform* platform) {
        this->platform = platform;
    };
    virtual ~Game() = default;

    virtual void Start() = 0;
    virtual void Update() = 0;

protected:
    Platform* platform;
};

class Scene {
public:
    explicit Scene(Platform* platform) {
        this->platform = platform;
    };
    virtual ~Scene() = default;

    virtual void Start() = 0;
    virtual void Update() = 0;

    int nextScene = -1;

protected:
    Platform* platform;
};

#endif //GAMEENGINE_GAME_H
