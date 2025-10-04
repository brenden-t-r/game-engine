#ifndef GAMEENGINE_TEXTURE_H
#define GAMEENGINE_TEXTURE_H

class Texture {
public:
    Texture(const char * path): path(path) {}
    virtual ~Texture() = default;
    const char* path;
};

#endif // GAMEENGINE_TEXTURE_H