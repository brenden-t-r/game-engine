
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

class Game {
public:
    explicit Game(Platform* platform) {
        this->platform = platform;
    };
    ~Game() = default;

    void Start() {
        platform->LoadShaders();
    }

    void Update() {
        printf(".");
        platform->DrawTriangle();
    }

private:
    Platform* platform;
};

// Common logic for the application
int RealMain(Platform* platform) {
    printf("Hello from PlatformMain!\n");

    platform->Init();

    Game* game = new Game(platform);
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


