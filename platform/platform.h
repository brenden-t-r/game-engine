#ifndef GAMEENGINE_PLATFORM_H
#define GAMEENGINE_PLATFORM_H

#include <iostream>
#include <functional>
#include "../engine/file.h"

class Platform {
public:
    Platform() = default;
    virtual ~Platform() = default;

    virtual void Init() = 0;
    virtual void LoadShaders() = 0;
    virtual void Run(const std::function<void()>& func) = 0;
    virtual void Shutdown() = 0;
};

class Game {
public:
    explicit Game(Platform* platform) {
        this->platform = platform;
    };
    ~Game() = default;

    void Start() {
        platform->LoadShaders();
    }

    void Update() {}

private:
    Platform* platform;
};

// Common logic for the application
int RealMain(Platform* platform) {
    std::cout << "Hello from PlatformMain!" << std::endl;

    platform->Init();

    Game* game = new Game(platform);
    game->Start();

    std::cout << "Running..." << std::endl;
    platform->Run([&game]() { game->Update(); });

    std::cout << "Shutting down..." << std::endl;
    platform->Shutdown();

    std::cout << "Goodbye!" << std::endl;
    delete game;
    delete platform;
    return 0;
}


#endif //GAMEENGINE_PLATFORM_H
