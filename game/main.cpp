/*
 *      ___           ___           ___           ___
 *     /  /\         /  /\         /__/\         /  /\
 *    /  /:/_       /  /::\       |  |::\       /  /:/_
 *   /  /:/ /\     /  /:/\:\      |  |:|:\     /  /:/ /\
 *  /  /:/_/::\   /  /:/~/::\   __|__|:|\:\   /  /:/ /:/_
 * /__/:/__\/\:\ /__/:/ /:/\:\ /__/::::| \:\ /__/:/ /:/ /\
 * \  \:\ /~~/:/ \  \:\/:/__\/ \  \:\~~\__\/ \  \:\/:/ /:/
 *  \  \:\  /:/   \  \::/       \  \:\        \  \::/ /:/
 *   \  \:\/:/     \  \:\        \  \:\        \  \:\/:/
 *    \  \::/       \  \:\        \  \:\        \  \::/
 *     \__\/         \__\/         \__\/         \__\/
 *
 */

#include "../engine/platform/platform.h"
#include "../engine/engine/game.h"
#include "../engine/samples/spriteGame.h"
#include "../engine/entry.h"

class SampleGame : public Game {
public:
    using Game::Game;
    void Start() override {
        platform->LoadShaders();
    }
    void Update() override {}
};

int RealMain(Platform* platform) {
    platform->Init();
    Game* game = new SpriteGame(platform);
    game->Start();
    RunLoop(platform, game);
    platform->Shutdown();
    delete game;
    delete platform;
    return 0;
}
