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

enum GAME_TYPES{
    AUDIO_GAME, COLLISION_GAME, FONT_GAME,
    INTPUT_GAME, LOCAL_MOVEMENT_GAME, PONG_GAME,
    ROTATION_GAME, SPRITE_GAME, TRIANGLE_GAME
};
Game* LoadGame(GAME_TYPES type, Platform* platform) {
    switch (type) {
        case AUDIO_GAME: return new AudioGame(platform);
        case COLLISION_GAME: return new CollisionGame(platform);
        case FONT_GAME: return new FontGame(platform);
        case INTPUT_GAME: return new InputGame(platform);
        case LOCAL_MOVEMENT_GAME: return new LocalMovementGame(platform);
        case PONG_GAME: return new PongGame(platform);
        case ROTATION_GAME: return new RotationGame(platform);
        case SPRITE_GAME: return new SpriteGame(platform);
        case TRIANGLE_GAME: return new TriangleGame(platform);
    }
}

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
        Game* gameTemp = currentGame;
        platform->RemoveAllCallbacks();
        currentGame = nullptr;
        delete gameTemp;
        GAME_TYPES gameType = static_cast<GAME_TYPES>(gameIndex);
        currentGame = LoadGame(gameType, platform);
        currentGame->Start();
    }

private:
    Game* currentGame{};

    int gameIndex = -1;
    static const int TOTAL_GAMES = 9;
};

#endif // GAMEENGINE_REGRESSIONTESTGAME_H