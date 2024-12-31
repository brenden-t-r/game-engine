#ifndef GAMEENGINE_BLANKGAME_H
#define GAMEENGINE_BLANKGAME_H

#include "../engine/game.h"

class BlankGame : public Game {
public:
    using Game::Game;

    ~BlankGame() override {};

    void Start() override {
        platform->LoadShaders();
    }

    void Update() override {
    }

private:
};

#endif //GAMEENGINE_BLANKGAME_H
