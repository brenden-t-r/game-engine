#ifndef GAMEENGINE_ROTATIONGAME_H
#define GAMEENGINE_ROTATIONGAME_H

#include "../engine/game.h"

class RotationGame : public Game {
public:
    using Game::Game;

    ~RotationGame() override = default;

    void Start() override {
        platform->LoadShaders();

        spriteBg = (Sprite*)platform->CreateSprite("assets/sprites/background.png");
        spriteBg->transform.width = 2.0f;
        spriteBg->transform.height = 2.0f;
        spriteBg->SetScale({1, 1, 0});
        spriteBg->SetPosition({0, 0, 1});

        float referenceWidth = 1920;
        float referenceHeight = 1080;

        sprite = (Sprite*)platform->CreateSprite("assets/sprites/cardaction.png");
        sprite->transform.width = 1024.0f / referenceWidth;
        sprite->transform.height = 1024.0f / referenceHeight;
        sprite->SetPosition({0, 0, 0});
        sprite->SetScale({1.0, 1.0, 0});
        sprite->Rotate(45);
        sprite->transform.parent = &spriteBg->transform;

        spriteChild = (Sprite*)platform->CreateSprite("assets/sprites/CardScarlet.png");
        spriteChild->transform.width = 1024.0f / referenceWidth;
        spriteChild->transform.height = 1024.0f / referenceHeight;
        spriteChild->SetPosition({0.1, 0.3, 0});
        spriteChild->SetScale({1, 1, 0});
        spriteChild->transform.parent = &sprite->transform;

        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.2f;
        triangle->transform.height = 0.2f;
        triangle->SetScale({0.5, 0.5, 0});
        triangle->transform.parent = &spriteChild->transform;

        spriteToTransform = sprite;

        EnableCallback(MOUSE_RELEASED);
    }

    void Update() override {
        printf(".");

        if (platform->IsKeyPressed(KeyCode::Right) || platform->IsGamepadButtonPressed(GamepadButton::East)) {
            spriteToTransform->Rotate(1);
        }
        if (platform->IsKeyPressed(KeyCode::Left)|| platform->IsGamepadButtonPressed(GamepadButton::West)) {
            spriteToTransform->Rotate(-1);
        }
        if (platform->IsKeyPressed(KeyCode::Up)|| platform->IsGamepadButtonPressed(GamepadButton::North)) {
            spriteToTransform->transform.scale.x += 0.01;
            spriteToTransform->transform.scale.y += 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::Down)|| platform->IsGamepadButtonPressed(GamepadButton::South)) {
            if (spriteToTransform->transform.scale.x > 0.001) {
                spriteToTransform->transform.scale.x -= 0.01;
                spriteToTransform->transform.scale.y -= 0.01;
            }
        }

        if (platform->IsKeyPressed(KeyCode::W)|| platform->IsGamepadButtonPressed(GamepadButton::DUp)) {
            spriteToTransform->Translate({0,0.01,0});
        }
        if (platform->IsKeyPressed(KeyCode::S)|| platform->IsGamepadButtonPressed(GamepadButton::DDown)) {
            spriteToTransform->Translate({0,-0.01,0});
        }
        if (platform->IsKeyPressed(KeyCode::D)|| platform->IsGamepadButtonPressed(GamepadButton::DRight)) {
            spriteToTransform->Translate({0.01,0,0});
        }
        if (platform->IsKeyPressed(KeyCode::A)|| platform->IsGamepadButtonPressed(GamepadButton::DLeft)) {
            spriteToTransform->Translate({-0.01,0,0});
        }

        spriteBg->Update();
        sprite->Update();
        spriteChild->Update();
        triangle->Update();
    }

private:
    Triangle* triangle;
    Sprite* spriteBg;
    Sprite* sprite;
    Sprite* spriteChild;

    Sprite* spriteToTransform;

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        printf("0");
        if (spriteToTransform == sprite) {
            printf("1");
            spriteToTransform = spriteChild;
        } else {
            printf("2");
            spriteToTransform = sprite;
        }
    }

};

#endif //GAMEENGINE_ROTATIONGAME_H
