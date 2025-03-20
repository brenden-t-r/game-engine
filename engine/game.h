#ifndef GAMEENGINE_GAME_H
#define GAMEENGINE_GAME_H

#include "../platform/platform.h"

class Game {
public:
    explicit Game(Platform* platform) {
        this->platform = platform;
    }

    virtual ~Game() = default;

    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual void KeyReleasedCallback(KeyCode key) {}
    virtual void MouseReleasedCallback(MouseButton key) {}
    virtual void GamepadReleasedCallback(GamepadButton key) {}

    void EnableCallback(Callback callbackType) {
        switch (callbackType) {
            case Callback::KEY_RELEASED: platform->SetKeyReleasedCallback(KeyReleasedCallback, this);
            case Callback::MOUSE_RELEASED: platform->SetMouseReleasedCallback(MouseReleasedCallback, this);
            case Callback::GAMEPAD_RELEASED: platform->SetGamepadReleasedCallback(GamepadReleasedCallback, this);
        }
    }
    static void KeyReleasedCallback(KeyCode key, void* context) {
        auto _this = (Game*)context;
        _this->KeyReleasedCallback(key);
    }
    static void MouseReleasedCallback(MouseButton button, void* context) {
        auto _this = (Game*)context;
        _this->MouseReleasedCallback(button);
    }
    static void GamepadReleasedCallback(GamepadButton button, void* context) {
        auto _this = (Game*)context;
        _this->GamepadReleasedCallback(button);
    }

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
    virtual bool IsSceneChange() { return nextScene != -1; }

    int nextScene = -1;

protected:
    Platform* platform;
};

static void LoadNextScene(Platform* platform, Scene** currentScene, Scene* (*sceneSwitchFn)(Platform*, int)) {
    if ((*currentScene)->nextScene == -1) return;
    auto nextScene = sceneSwitchFn(platform, (*currentScene)->nextScene);
    nextScene->Start();
    auto temp = *currentScene;
    *currentScene = nextScene;
    delete temp;
}

#endif //GAMEENGINE_GAME_H
