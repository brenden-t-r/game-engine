#ifndef GAMEENGINE_PLATFORM_H
#define GAMEENGINE_PLATFORM_H

#include <iostream>

class Platform {
public:
    Platform() = default;
    virtual ~Platform() = default;

    virtual void Init() = 0;
    virtual void Run() = 0;
    virtual void Shutdown() = 0;
};

// Forward declare the common entry point
//int PlatformMain();

// Common logic for the application
int PlatformMain(Platform* platform) {
    std::cout << "Hello from PlatformMain!" << std::endl;

    platform->Init();
    std::cout << "Running..." << std::endl;
    platform->Run();
    std::cout << "Shutting down..." << std::endl;
    platform->Shutdown();

    std::cout << "Goodbye!" << std::endl;
    delete platform;
    return 0;
}

#endif //GAMEENGINE_PLATFORM_H
