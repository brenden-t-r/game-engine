#ifndef GAMEENGINE_BLANKSCENEGAME_H
#define GAMEENGINE_BLANKSCENEGAME_H

#include "../engine/game.h"

enum scenes {
    SCENE_MAIN,
    SCENE_2,
};

class BlankScene : public Scene {
    using Scene::Scene;
    ~BlankScene() override {
        delete gameObject;
    }
    void Start() override {
        gameObject = platform->CreateTriangle();
    }
    void Update() override {
        counter+=1;
        if (counter > 50) nextScene = SCENE_2;
        gameObject->Update();
    }
    GameObject* gameObject = nullptr;
    int counter = 0;
};

class BlankScene2 : public Scene {
    using Scene::Scene;
    ~BlankScene2() override {
        delete gameObject;
    }
    void Start() override {
        gameObject = platform->CreateTriangle();
        gameObject->transform.pos.x += 0.5f;
    }
    void Update() override {
        counter+=1;
        if (counter > 50) nextScene = SCENE_MAIN;
        gameObject->Update();
    }
    GameObject* gameObject = nullptr;
    int counter = 0;
};

static Scene* GetSceneFn(GamePlatform* platform, int sceneToLoad) {
    switch (sceneToLoad) {
        case SCENE_MAIN: return new BlankScene(platform);
        case SCENE_2: return new BlankScene2(platform);
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
        platform->LoadShaders();
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
