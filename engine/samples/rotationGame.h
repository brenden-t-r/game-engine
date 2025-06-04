#ifndef GAMEENGINE_ROTATIONGAME_H
#define GAMEENGINE_ROTATIONGAME_H

#include "../engine/game.h"

class RotationGame : public Game {
public:
    using Game::Game;

    ~RotationGame() override = default;

    void Start() override {
        platform->LoadShaders();

        sprite = (Sprite*)platform->CreateSprite("assets/sprites/background.png");
        sprite->transform.width = 0.5f;
        sprite->transform.height = 0.5f;
        sprite->transform.rot = {0,0,45};
        sprite->SetPosition(screen_to_normalized({960, 500, 0}));
        sprite->SetScale({2.0, 2.0, 0});

        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.2f;
        triangle->transform.height = 0.2f;
        triangle->transform.rot = {0,0,45};
        triangle->SetPosition(screen_to_normalized({300, 500, 0}));
        triangle->SetScale({2.0, 2.0, 0});
    }

    void Update() override {
        printf(".");

        if (platform->IsKeyPressed(KeyCode::Right) || platform->IsGamepadButtonPressed(GamepadButton::East)) {
            sprite->Rotate(1);
            triangle->Rotate(1);
        }
        if (platform->IsKeyPressed(KeyCode::Left)|| platform->IsGamepadButtonPressed(GamepadButton::West)) {
            sprite->Rotate(-1);
            triangle->Rotate(-1);
        }
        if (platform->IsKeyPressed(KeyCode::Up)|| platform->IsGamepadButtonPressed(GamepadButton::North)) {
            sprite->transform.scale.x += -0.1;
            sprite->transform.scale.y += -0.1;
        }
        if (platform->IsKeyPressed(KeyCode::Down)|| platform->IsGamepadButtonPressed(GamepadButton::South)) {
            sprite->transform.scale.x -= -0.1;
            sprite->transform.scale.y -= -0.1;
        }

        if (platform->IsKeyPressed(KeyCode::W)|| platform->IsGamepadButtonPressed(GamepadButton::DUp)) {
            sprite->Translate({0,0.01,0});
            triangle->Translate({0,0.01,0});
        }
        if (platform->IsKeyPressed(KeyCode::S)|| platform->IsGamepadButtonPressed(GamepadButton::DDown)) {
            sprite->Translate({0,-0.01,0});
            triangle->Translate({0,-0.01,0});
        }
        if (platform->IsKeyPressed(KeyCode::D)|| platform->IsGamepadButtonPressed(GamepadButton::DRight)) {
            sprite->Translate({0.01,0,0});
            triangle->Translate({0.01,0,0});
        }
        if (platform->IsKeyPressed(KeyCode::A)|| platform->IsGamepadButtonPressed(GamepadButton::DLeft)) {
            sprite->Translate({-0.01,0,0});
            triangle->Translate({-0.01,0,0});
        }

//        triangle->Update();
        sprite->Update();
    }

private:
    Triangle* triangle;
    Sprite* sprite;
};

#endif //GAMEENGINE_ROTATIONGAME_H