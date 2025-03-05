#ifndef GAMEENGINE_PLATFORM_H
#define GAMEENGINE_PLATFORM_H

#include "../engine/gameobject.h"

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

enum Callback {
    KEY_RELEASED,
    MOUSE_RELEASED,
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
    virtual void SetMouseReleasedCallback(void (*func)(MouseButton, void*), void* context) = 0;
    virtual void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) = 0;
    virtual vector3 GetMousePos() = 0;

    virtual void Shutdown() = 0;

    // Helper functions

    GameObject* CreateTriangle(vector3 pos, float width, float height) {
        auto object = CreateTriangle();
        object->transform.pos = pos;
        object->transform.width = width;
        object->transform.height = height;
        return object;
    }
    GameObject* CreateTriangle(float posX, float posY, float width, float height) {
        auto object = CreateTriangle();
        object->transform.pos = { posX, posY, 0 };
        object->transform.width = width;
        object->transform.height = height;
        return object;
    }
};

#endif //GAMEENGINE_PLATFORM_H
