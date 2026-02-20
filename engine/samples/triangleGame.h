#ifndef GAMEENGINE_TRIANGLEGAME_H
#define GAMEENGINE_TRIANGLEGAME_H

#include "../engine/game.h"

class TriangleGame : public Game {
public:
    using Game::Game;

    ~TriangleGame() override {
        platform->Delete(triangle);
    };

    void Start() override {
        platform->LoadShaders();
        triangle = platform->CreateTriangle();
    }

    void Update() override {
        printf(".");
        triangle->Update();
        triangle->transform.pos.x += 0.01f;
    }

private:
    GameObject* triangle{};
};

#endif //GAMEENGINE_TRIANGLEGAME_H
