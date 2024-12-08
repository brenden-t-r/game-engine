
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

class SampleGame : public Game {
public:
    using Game::Game;

    ~SampleGame() override {
        delete triangle;
    };

    void Start() override {
        platform->LoadShaders();
    }

    void Update() override {
        printf(".");
    }

private:
    GameObject* triangle{};
};

// Common logic for the application
int RealMain(Platform* platform) {
    printf("Hello from PlatformMain!\n");

    platform->Init();

//    Game* game = new SampleGame(platform);
    Game* game = new TriangleGame(platform);
    game->Start();

    printf("Running...\n");
    platform->Run([&game]() { game->Update(); });

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
#if defined(PLATFORM_WINDOWS) and defined(BACKEND_OPENGL)
#include "platform/opengl.h"
#elif defined(PLATFORM_WINDOWS) and defined(BACKEND_DIRECTX)
#include "platform/directx/directx.h"
#elif defined(PLATFORM_LINUX)
#include "platform/opengl.h"
#elif defined(PLATFORM_APPLE)
#else
// No backend selected
int main() {
    return -1;
}
#endif
