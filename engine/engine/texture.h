#ifndef GAMEENGINE_TEXTURE_H
#define GAMEENGINE_TEXTURE_H

#include "resource.h"

class Texture {
public:
    virtual ~Texture() = default;
    const char* path;
};

#endif // GAMEENGINE_TEXTURE_H