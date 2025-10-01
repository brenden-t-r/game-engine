#ifndef GAMEENGINE_RESOURCE_H
#define GAMEENGINE_RESOURCE_H

#include "../platform/platform.h"

#include <unordered_map>
#include <string>

template<typename T>
class ResourceCache {
public:
    std::unordered_map<std::string, T*> map;

    T* Get(const std::string& path) {
        auto it = map.find(path);
        return (it != map.end()) ? it->second : nullptr;
    }

    void Add(const std::string& path, T* resource) {
        map[path] = resource;
    }

    void Release(const std::string& path) {
        auto it = map.find(path);
        if (it == map.end()) return;
        delete it->second;
        map.erase(it);
    }
};

class TextureCache {
public:
    std::unordered_map<std::string, Texture*> map;

    Texture* Get(const std::string& path) {
        auto it = map.find(path);
        return (it != map.end()) ? it->second : nullptr;
    }

    Texture* GetOrLoad(Platform* platform, const std::string& path) {
        auto it = map.find(path);
        if (it != map.end()) {
            return it->second;
        }
        auto texture = platform->CreateTexture(path.c_str());
        map[path] = texture;
        return texture;
    }

    void Add(const std::string& path, Texture* resource) {
        map[path] = resource;
    }

    void Release(const std::string& path) {
        auto it = map.find(path);
        if (it == map.end()) return;
        delete it->second;
        map.erase(it);
    }
};


#endif // GAMEENGINE_RESOURCE_H