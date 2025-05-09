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
//        sprite->transform.rot = {0,0,45};
        sprite->SetPosition(screen_to_normalized({960, 500, 0}));

        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.2f;
        triangle->transform.height = 0.2f;
        triangle->transform.rot = {0,0,45};
        triangle->SetPosition(screen_to_normalized({300, 500, 0}));

        EnableCallback(KEY_RELEASED);
    }

    void Update() override {
        printf(".");

        if (platform->IsKeyPressed(KeyCode::W)) {
            sprite->Rotate(1);
            triangle->Rotate(1);
        }
        if (platform->IsKeyPressed(KeyCode::S)) {
            sprite->Rotate(-1);
            triangle->Rotate(-1);
        }

        if (platform->IsKeyPressed(KeyCode::Up)) {
            sprite->Translate({0,0.01,0});
            triangle->Translate({0,0.01,0});
        }
        if (platform->IsKeyPressed(KeyCode::Down)) {
            sprite->Translate({0,-0.01,0});
            triangle->Translate({0,-0.01,0});
        }
        if (platform->IsKeyPressed(KeyCode::Right)) {
            sprite->Translate({0.01,0,0});
            triangle->Translate({0.01,0,0});
        }
        if (platform->IsKeyPressed(KeyCode::Left)) {
            sprite->Translate({-0.01,0,0});
            triangle->Translate({-0.01,0,0});
        }

        triangle->Update();
        sprite->Update();
    }

private:
    Triangle* triangle;
    Sprite* sprite;

    void KeyReleasedCallback(KeyCode key) override {
        if (key == KeyCode::D) {
            triangle->Rotate(45);
            sprite->Rotate(45);
        }
        if (key == KeyCode::A) {
            triangle->Rotate(-45);
            sprite->Rotate(-45);
        }
    }
};

#endif //GAMEENGINE_ROTATIONGAME_H