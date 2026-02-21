#ifndef GAMEENGINE_PLATFORM_H
#define GAMEENGINE_PLATFORM_H

#include "../engine/gameobject.h"
#include "../engine/texture.h"
#include "../engine/material.h"

//region: Enums
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
enum class GamepadButton {
    Unknown = 0,
    North, South, East, West,
    RB, LB, R3, L3, Start, Select,
    DLeft, DRight, DUp, DDown
};
enum Callback {
    KEY_RELEASED,
    MOUSE_RELEASED,
    GAMEPAD_RELEASED
};
//endregion

class Platform {
public:
    Platform() = default;
    virtual ~Platform() = default;

    virtual void Init() = 0;
    virtual void LoadShaders() = 0;
    virtual Shader* LoadShader(ShaderDef shaderDef) = 0;
    virtual void Run(void (*func)(void*), void* context) = 0;

    virtual GameObject* CreateGameObject(){ return new GameObject(); };
    virtual GameObject* CreateTriangle() = 0;
    virtual Sprite* CreateSprite(const char* path) = 0;
    virtual Sprite* CreateSprite(Texture* texture) = 0;
    virtual Texture* CreateTexture(const char* path) = 0;
    virtual Sound* CreateSound(const char* path) = 0;
    virtual void Delete(GameObject* object) {
        delete object;
    }

    virtual bool IsKeyPressed(KeyCode key) = 0;
    virtual bool IsMousePressed(MouseButton button) = 0;
    virtual bool IsGamepadButtonPressed(GamepadButton button) = 0;
    virtual void SetGamepadVibration(int amountLeft, int amountRight){};
    virtual void SetKeyReleasedCallback(void (*func)(KeyCode, void*), void* context) = 0;
    virtual void SetMouseReleasedCallback(void (*func)(MouseButton, void*, vec3), void* context) = 0;
    virtual void SetGamepadReleasedCallback(void (*func)(GamepadButton, void*), void* context) = 0;
    virtual void RemoveAllCallbacks() = 0;
    virtual vec3 GetMousePos() = 0;


    virtual void Shutdown() = 0;

    // Helper functions
    GameObject* CreateTriangle(float posX, float posY, float width, float height) {
        auto object = CreateTriangle();
        object->transform.pos = { posX, posY, 0 };
        object->transform.width = width;
        object->transform.height = height;
        return object;
    }
    Sprite* CreateSprite(const char* path, float posX, float posY, float width, float height) {
        auto object = CreateSprite(path);
        object->transform.pos = { posX, posY, 0 };
        object->transform.width = width;
        object->transform.height = height;
        return object;
    }
    Sprite* CreateSpriteAtlas(const char* path, float posX, float posY, float width, float height,
                              int atlasNumRows, float atlasCellSize) {
        auto object = CreateSprite(path, posX, posY, width, height);
        object->transform.pos = { posX, posY, 0 };
        object->useAtlas = true;
        object->atlasNumRows = atlasNumRows;
        object->atlasCellSize = atlasCellSize;
        object->atlasRow = 0;
        object->atlasColumn = 0;
        return object;
    }
};

#endif //GAMEENGINE_PLATFORM_H
