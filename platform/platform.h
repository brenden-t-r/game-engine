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
    virtual void DrawTriangle() = 0;
    virtual void Shutdown() = 0;
};


#endif //GAMEENGINE_PLATFORM_H
