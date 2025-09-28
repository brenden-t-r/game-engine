#ifndef GAMEENGINE_SPRITEGAME_H
#define GAMEENGINE_SPRITEGAME_H

#include "../engine/game.h"

class SpriteGame : public Game {
public:
    using Game::Game;

    ~SpriteGame() override {
        delete triangle;
        delete sprite;
        delete triangle2;
        delete sprite2;
    };

    void Start() override {
        platform->LoadShaders();

        sprite = platform->CreateSprite("assets/sprites/background.png");
        sprite->transform.width = 2.0f;
        sprite->transform.height = 2.0f;
        triangle = platform->CreateTriangle();

        sprite2 = platform->CreateSprite("assets/sprites/background.png");
        sprite2->transform.width = 2.0f;
        sprite2->transform.height = 2.0f;
        triangle2 = platform->CreateTriangle();
    }

    void Update() override {
        printf(".");

        platform->SetViewport(0, WINDOW_HEIGHT/2.0, WINDOW_WIDTH, WINDOW_HEIGHT);
        sprite->Update();
        sprite2->Update();
        triangle->Update();
        triangle2->Update();
        platform->SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT/2.0, WINDOW_WIDTH, WINDOW_HEIGHT);
        sprite->Update();
        sprite2->Update();
        triangle->Update();
        triangle2->Update();


//        triangle->transform.pos.x += 0.01f;
//        sprite->transform.pos.x += 0.01f;
        //triangle2->transform.pos.x -= 0.01f;
        //sprite2->transform.pos.x -= 0.01f;
    }

private:
    GameObject* sprite{};
    GameObject* triangle{};
    GameObject* sprite2{};
    GameObject* triangle2{};
};

#endif //GAMEENGINE_SPRITEGAME_H
