
/*
 * Choose backend
 *  Windows => OpenGL, DirectX
 *  Linux   => OpenGL
 *  macOS   => OpenGL, Metal
 *  iOS     => Metal
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

        sprite = (Sprite*)platform->CreateSprite("assets/sprites/background.png");
        sprite->transform.width = 0.5f;
        sprite->transform.height = 0.5f;
        sprite->transform.pos = coords_screen_to_device(startPos);;
        sprite->vertex1.x = sprite->transform.pos.x - sprite->transform.width/2;
        sprite->vertex1.y = sprite->transform.pos.y + sprite->transform.height/2;
        sprite->vertex2.x = sprite->transform.pos.x + sprite->transform.width/2;
        sprite->vertex2.y = sprite->transform.pos.y + sprite->transform.height/2;
        sprite->vertex3.x = sprite->transform.pos.x + sprite->transform.width/2;
        sprite->vertex3.y = sprite->transform.pos.y - sprite->transform.height/2;
        sprite->vertex4.x = sprite->transform.pos.x - sprite->transform.width/2;
        sprite->vertex4.y = sprite->transform.pos.y - sprite->transform.height/2;

        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.2f;
        triangle->transform.height = 0.2f;
        triangle->transform.pos = coords_screen_to_device({300, 500, 0});
        triangle->vertex1.x = triangle->transform.pos.x + triangle->transform.width / 2;
        triangle->vertex1.y = triangle->transform.pos.y - triangle->transform.height / 2;
        triangle->vertex2.x = triangle->transform.pos.x - triangle->transform.width / 2;
        triangle->vertex2.y = triangle->transform.pos.y - triangle->transform.height / 2;
        triangle->vertex3.x = triangle->transform.pos.x;
        triangle->vertex3.y = triangle->transform.pos.y + triangle->transform.height / 2;

        EnableCallback(KEY_RELEASED);
    }

    vec3 startPos = {960, 500, 0};

    static void Rotate(Sprite* obj, float degrees) {
        // "Undo" current position transform back to screen space origin (top left 0,0)
        vec3 posScreenSpace = coords_device_to_screen(obj->transform.pos);

        obj->vertex1.x = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x - posScreenSpace.x, coords_device_to_screen(obj->vertex1).y - posScreenSpace.y, 0}).x;
        obj->vertex2.x = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x - posScreenSpace.x, coords_device_to_screen(obj->vertex2).y - posScreenSpace.y, 0}).x;
        obj->vertex3.x = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x - posScreenSpace.x, coords_device_to_screen(obj->vertex3).y - posScreenSpace.y, 0}).x;
        obj->vertex4.x = coords_screen_to_device({coords_device_to_screen(obj->vertex4).x - posScreenSpace.x, coords_device_to_screen(obj->vertex4).y - posScreenSpace.y, 0}).x;
        obj->vertex1.y = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x - posScreenSpace.x, coords_device_to_screen(obj->vertex1).y - posScreenSpace.y, 0}).y;
        obj->vertex2.y = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x - posScreenSpace.x, coords_device_to_screen(obj->vertex2).y - posScreenSpace.y, 0}).y;
        obj->vertex3.y = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x - posScreenSpace.x, coords_device_to_screen(obj->vertex3).y - posScreenSpace.y, 0}).y;
        obj->vertex4.y = coords_screen_to_device({coords_device_to_screen(obj->vertex4).x - posScreenSpace.x, coords_device_to_screen(obj->vertex4).y - posScreenSpace.y, 0}).y;

        obj->vertex1 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex1), degrees));
        obj->vertex2 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex2), degrees));
        obj->vertex3 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex3), degrees));
        obj->vertex4 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex4), degrees));

        obj->vertex1.x = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x + posScreenSpace.x, coords_device_to_screen(obj->vertex1).y + posScreenSpace.y, 0}).x;
        obj->vertex2.x = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x + posScreenSpace.x, coords_device_to_screen(obj->vertex2).y + posScreenSpace.y, 0}).x;
        obj->vertex3.x = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x + posScreenSpace.x, coords_device_to_screen(obj->vertex3).y + posScreenSpace.y, 0}).x;
        obj->vertex4.x = coords_screen_to_device({coords_device_to_screen(obj->vertex4).x + posScreenSpace.x, coords_device_to_screen(obj->vertex4).y + posScreenSpace.y, 0}).x;
        obj->vertex1.y = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x + posScreenSpace.x, coords_device_to_screen(obj->vertex1).y + posScreenSpace.y, 0}).y;
        obj->vertex2.y = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x + posScreenSpace.x, coords_device_to_screen(obj->vertex2).y + posScreenSpace.y, 0}).y;
        obj->vertex3.y = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x + posScreenSpace.x, coords_device_to_screen(obj->vertex3).y + posScreenSpace.y, 0}).y;
        obj->vertex4.y = coords_screen_to_device({coords_device_to_screen(obj->vertex4).x + posScreenSpace.x, coords_device_to_screen(obj->vertex4).y + posScreenSpace.y, 0}).y;
    }

    static void Rotate(Triangle* obj, float degrees) {
        // "Undo" current position transform back to screen space origin (top left 0,0)
        vec3 posScreenSpace = coords_device_to_screen(obj->transform.pos);

        obj->vertex1.x = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x - posScreenSpace.x, coords_device_to_screen(obj->vertex1).y - posScreenSpace.y, 0}).x;
        obj->vertex2.x = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x - posScreenSpace.x, coords_device_to_screen(obj->vertex2).y - posScreenSpace.y, 0}).x;
        obj->vertex3.x = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x - posScreenSpace.x, coords_device_to_screen(obj->vertex3).y - posScreenSpace.y, 0}).x;
        obj->vertex1.y = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x - posScreenSpace.x, coords_device_to_screen(obj->vertex1).y - posScreenSpace.y, 0}).y;
        obj->vertex2.y = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x - posScreenSpace.x, coords_device_to_screen(obj->vertex2).y - posScreenSpace.y, 0}).y;
        obj->vertex3.y = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x - posScreenSpace.x, coords_device_to_screen(obj->vertex3).y - posScreenSpace.y, 0}).y;

        obj->vertex1 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex1), degrees));
        obj->vertex2 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex2), degrees));
        obj->vertex3 = coords_screen_to_device(rotate_euler(coords_device_to_screen(obj->vertex3), degrees));

        obj->vertex1.x = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x + posScreenSpace.x, coords_device_to_screen(obj->vertex1).y + posScreenSpace.y, 0}).x;
        obj->vertex2.x = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x + posScreenSpace.x, coords_device_to_screen(obj->vertex2).y + posScreenSpace.y, 0}).x;
        obj->vertex3.x = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x + posScreenSpace.x, coords_device_to_screen(obj->vertex3).y + posScreenSpace.y, 0}).x;
        obj->vertex1.y = coords_screen_to_device({coords_device_to_screen(obj->vertex1).x + posScreenSpace.x, coords_device_to_screen(obj->vertex1).y + posScreenSpace.y, 0}).y;
        obj->vertex2.y = coords_screen_to_device({coords_device_to_screen(obj->vertex2).x + posScreenSpace.x, coords_device_to_screen(obj->vertex2).y + posScreenSpace.y, 0}).y;
        obj->vertex3.y = coords_screen_to_device({coords_device_to_screen(obj->vertex3).x + posScreenSpace.x, coords_device_to_screen(obj->vertex3).y + posScreenSpace.y, 0}).y;
    }

    void Update() override {
        printf(".");

        if (platform->IsKeyPressed(KeyCode::Up)) {
            Rotate(sprite, 1);
            Rotate(triangle, 1);
        }
        if (platform->IsKeyPressed(KeyCode::Down)) {
            Rotate(sprite, -1);
            Rotate(triangle, -1);
        }

        if (platform->IsKeyPressed(KeyCode::Right)) {
            sprite->vertex1.x += 0.01;
            sprite->vertex2.x += 0.01;
            sprite->vertex3.x += 0.01;
            sprite->vertex4.x += 0.01;
            sprite->transform.pos.x += 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::Left)) {
            sprite->vertex1.x -= 0.01;
            sprite->vertex2.x -= 0.01;
            sprite->vertex3.x -= 0.01;
            sprite->vertex4.x -= 0.01;
            sprite->transform.pos.x -= 0.01;
        }

        triangle->Update();
        sprite->Update();
    }

private:
    Triangle* triangle;
    Sprite* sprite;

    void KeyReleasedCallback(KeyCode key) override {
        if (key == KeyCode::Down) {
            Rotate(sprite, 45);
            Rotate(triangle, 45);
        }
    }
};

void GameUpdateFn(void* context) {
    ((Game*)(context))->Update();
}

int RealMain(Platform* platform) {
    printf("Hello from PlatformMain!\n");

    platform->Init();

    Game* game = new SampleGame(platform);
//    Game* game = new TriangleGame(platform);
//    Game* game = new SpriteGame(platform);
//    Game* game = new PongGame(platform);
//    Game* game = new AudioGame(platform);
//    Game* game = new InputGame(platform);
//    Game* game = new BLANK_GAME::BlankSceneGame(platform);
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
#elif defined(PLATFORM_IOS)
#else
// No backend selected
int main() {
    return -1;
}
#endif
