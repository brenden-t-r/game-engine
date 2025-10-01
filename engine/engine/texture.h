#ifndef GAMEENGINE_TEXTURE_H
#define GAMEENGINE_TEXTURE_H

enum class TextureFilter {
    LINEAR = 0,
    POINT = 1
};

struct TextureSettings {
    TextureFilter filter;
//    bool mipMaps;
};

TextureSettings DEFAULT_TEXTURE_SETTINGS = TextureSettings{TextureFilter::LINEAR};

class Texture {
public:
    Texture(const char* path, TextureSettings settings): path(path), settings(settings) {}
    virtual ~Texture() = default;

    const char* path{};
    const TextureSettings settings;
};

#endif // GAMEENGINE_TEXTURE_H