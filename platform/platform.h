#ifndef GAMEENGINE_PLATFORM_H
#define GAMEENGINE_PLATFORM_H

#include "../engine/gameobject.h"

enum KeyCode {
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
    virtual void Run(void (*func)(void*), void* context) = 0;

    virtual GameObject* CreateGameObject(){ return new GameObject(); };
    virtual GameObject* CreateTriangle() = 0;
    virtual Sprite* CreateSprite(const char* path) = 0;

    virtual bool IsKeyPressed(KeyCode key) = 0;
    virtual bool IsKeyReleased(KeyCode key) = 0;
    virtual bool IsMousePressed(MouseButton button) = 0;
    virtual bool IsMouseReleased(MouseButton button) = 0;
    virtual vector3 GetMousePos() = 0;

    virtual void Shutdown() = 0;
};

#endif //GAMEENGINE_PLATFORM_H
