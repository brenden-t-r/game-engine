#ifndef GAMEENGINE_BLANKSCENEGAME_H
#define GAMEENGINE_BLANKSCENEGAME_H

#include "../engine/game.h"

enum scenes {
    SCENE_MAIN,
};

class BlankScene : public Scene {
    using Scene::Scene;
    void Start() override {}
    void Update() override {}
};

static Scene* GetSceneFn(Platform* platform, int sceneToLoad) {
    switch (sceneToLoad) {
        case SCENE_MAIN: return new BlankScene(platform);
        default: return nullptr;
    }
}

class BlankSceneGame : public Game {
public:
    using Game::Game;

    ~BlankSceneGame() override {
        delete scene;
    };

    void Start() override {
        scene = new BlankScene(platform);
        scene->Start();
    }

    void Update() override {
        if (scene->IsSceneChange()) {
            LoadNextScene(platform, &scene, GetSceneFn);
        }
        scene->Update();
    }

private:
    Scene* scene = nullptr;
};


#endif //GAMEENGINE_BLANKSCENEGAME_H
