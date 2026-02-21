#ifndef GAMEENGINE_GAMELOADER_H
#define GAMEENGINE_GAMELOADER_H

#include "../engine/game.h"
#include "../engine/resource.h"
#include "spriteGame.h"

class GameLoaderGame : public Game {
public:
    using Game::Game;
    void Start() override {
        platform->LoadShaders();
        EnableCallback(Callback::KEY_RELEASED);
    }
    void Update() override {
        if (game != nullptr) {
            game->Update();
        }
    }

    void KeyReleasedCallback(KeyCode key) override {
        if (key == KeyCode::Up && game == nullptr) {
            game = new SpriteGame(platform);
            game->Start();
        }
        if (key == KeyCode::Down && game != nullptr) {
            Game* gameTemp = game;
            game = nullptr;
            delete gameTemp;
        }
    }
private:
    Game* game{};
};

#endif // GAMEENGINE_GAMELOADER_H