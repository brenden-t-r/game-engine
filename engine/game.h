#ifndef GAMEENGINE_GAME_H
#define GAMEENGINE_GAME_H

#include "../platform/platform.h"

class Game {
public:
    explicit Game(GamePlatform* platform) {
        this->platform = platform;
    };
    virtual ~Game() = default;

    virtual void Start() = 0;
    virtual void Update() = 0;

protected:
    GamePlatform* platform;
};

class Scene {
public:
    explicit Scene(GamePlatform* platform) {
        this->platform = platform;
    };
    virtual ~Scene() = default;

    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual bool IsSceneChange() { return nextScene != -1; }

    int nextScene = -1;

protected:
    GamePlatform* platform;
};

static void LoadNextScene(GamePlatform* platform, Scene** currentScene, Scene* (*sceneSwitchFn)(GamePlatform*, int)) {
    if ((*currentScene)->nextScene == -1) return;
    auto nextScene = sceneSwitchFn(platform, (*currentScene)->nextScene);
    nextScene->Start();
    auto temp = *currentScene;
    *currentScene = nextScene;
    delete temp;
}

#endif //GAMEENGINE_GAME_H
