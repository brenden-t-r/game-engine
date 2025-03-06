
/*
 * Choose backend
 *  Windows => OpenGL, DirectX
 *  Linux   => OpenGL
 *  Apple   => OpenGL, Metal
 * */
//#define BACKEND_DIRECTX
//#define BACKEND_OPENGL
//#define BACKEND_METAL

#include "platform/platform.h"
#include "engine/game.h"
#include "samples/triangleGame.h"
#include "samples/spriteGame.h"
#include "samples/pongGame.h"
#include "samples/inputGame.h"
#include "samples/audioGame.h"
#include "samples/blankSceneGame.h"
#include "samples/fontGame.h"
#include "samples/collisionGame.h"

class SampleGame : public Game {
public:
    using Game::Game;

    ~SampleGame() override = default;

    void Start() override {
        platform->LoadShaders();
    }

    void Update() override {
        printf(".");
    }

private:
};

void GameUpdateFn(void* context) {
    ((Game*)(context))->Update();
}

int RealMain(Platform* platform) {
    printf("Hello from PlatformMain!\n");

    platform->Init();

//    Game* game = new SampleGame(platform);
//    Game* game = new TriangleGame(platform);
//    Game* game = new SpriteGame(platform);
    Game* game = new PongGame(platform);
//    Game* game = new AudioGame(platform);
//    Game* game = new InputGame(platform);
//    Game* game = new BlankSceneGame(platform);
//    Game* game = new FontGame(platform);
//    Game* game = new CollisionGame(platform);
    game->Start();

    printf("Running...\n");

    platform->Run(GameUpdateFn, game);

    printf("\nShutting down...\n");
    platform->Shutdown();

    printf("Goodbye!\n");
    delete game;
    delete platform;
    return 0;
}

/*
 * Platform-specific entry-points using preprocessor macro
 */
#if defined(PLATFORM_WINDOWS) && defined(BACKEND_OPENGL)
#include "platform/opengl/opengl.h"
#elif defined(PLATFORM_WINDOWS) && defined(BACKEND_DIRECTX)
#include "platform/directx/directx.h"
#elif defined(PLATFORM_LINUX)
#include "platform/opengl/opengl.h"
#elif defined(PLATFORM_APPLE) && defined(BACKEND_OPENGL)
#include "platform/opengl/opengl.h"
#elif defined(PLATFORM_APPLE) && defined(BACKEND_METAL)
#else
// No backend selected
int main() {
    return -1;
}
#endif
