#ifndef GAMEENGINE_REGRESSIONTESTGAME_H
#define GAMEENGINE_REGRESSIONTESTGAME_H

#include "../engine/game.h"
#include "../engine/resource.h"
#include "audioGame.h"
#include "collisionGame.h"
#include "fontGame.h"
#include "inputGame.h"
#include "localMovementGame.h"
#include "pongGame.h"
#include "rotationGame.h"
#include "spriteGame.h"
#include "triangleGame.h"

class RegressionTestGame : public Game {
public:
    using Game::Game;
    void Start() override {
        platform->LoadShaders();
    }

    int counter = 0;
    void Update() override {
        if (currentGame != nullptr) {
            currentGame->Update();
        }
        if (platform->IsMousePressed(MouseButton::Right)) {
            counter += 1;
            if (counter == 50) {
                counter = 0;
                NextGame();
            }
        }
    }

    void NextGame() {
        if (gameIndex >= TOTAL_GAMES - 1) {
            gameIndex = 0;
        } else gameIndex += 1;
        currentGame = games[gameIndex];
        currentGame->Start();
    }

private:
    Game* currentGame{};

    int gameIndex = -1;
    static const int TOTAL_GAMES = 9;
    Game* games[TOTAL_GAMES] = {
        new AudioGame(platform),
        new CollisionGame(platform),
        new FontGame(platform),
        new InputGame(platform),
        new LocalMovementGame(platform),
        new PongGame(platform),
        new RotationGame(platform),
        new SpriteGame(platform),
        new TriangleGame(platform)
    };
};

#endif // GAMEENGINE_REGRESSIONTESTGAME_H