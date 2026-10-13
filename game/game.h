#include "engine/game.h"

class IsoGame : public Game {
public:
    using Game::Game;

    void Start() override {
        platform->LoadShaders();
    }

    void Update() override {

    }
};