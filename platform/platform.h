#ifndef GAMEENGINE_PLATFORM_H
#define GAMEENGINE_PLATFORM_H

#include <iostream>
#include <functional>
#include "../engine/file.h"
#include "../engine/vector.h"

enum class KeyCode {
    Unknown = 0,
    Up,
    Down,
    Left,
    Right,
    W,
    A,
    S,
    D,
    // Add more keys as needed
};

enum class MouseButton {
    Unknown = 0,
    Left,
    Middle,
    Right
};


class Platform {
public:
    Platform() = default;
    virtual ~Platform() = default;

    virtual void Init() = 0;
    virtual void LoadShaders() = 0;
    virtual void Run(const std::function<void()>& func) = 0;

    virtual GameObject* CreateGameObject(){ return new GameObject(); };
    virtual GameObject* CreateTriangle() = 0;
    virtual Sprite* CreateSprite(const char* path) = 0;

    virtual bool IsKeyPressed(KeyCode key) = 0;
    virtual bool IsMousePressed(MouseButton button) = 0;
    virtual bool IsMouseReleased(MouseButton button) = 0;
    virtual Vector3 GetMousePos() = 0;

    virtual void Shutdown() = 0;
};

#endif //GAMEENGINE_PLATFORM_H
