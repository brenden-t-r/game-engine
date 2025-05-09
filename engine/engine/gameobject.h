#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include "../constants.h"
#include "vector.h"

#include <cstdio>
#include <vector>

class Transform {
public:
    vec3 pos = {0, 0, 0};
    vec3 rot = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    float width = 1;
    float height = 1;

    void Translate(vec3 vector){}
    void Rotate(float degrees){}
};

class GameObject {
public:
    Transform transform{};
    virtual ~GameObject(){}
    virtual void Start(){}
    virtual void Update(){}
};

class Triangle : public GameObject {
public:
    vec3 vertex1 = {1.0f, -1.0f, 0.0f};
    vec3 vertex2 = {-1.0f, -1.0f, 0.0f};
    vec3 vertex3 = {0, 1.0f, 0.0f};

    void Start() override {
        vertex1.x = transform.pos.x + transform.width/2;
        vertex1.y = transform.pos.y - transform.height/2;
        vertex2.x = transform.pos.x - transform.width/2;
        vertex2.y = transform.pos.y - transform.height/2;
        vertex3.x = transform.pos.x;
        vertex3.y = transform.pos.y + transform.height/2;
    }

    void Update() override {
        GameObject::Update();
    }
};

class Sprite : public GameObject {
public:
    vec3 vertex1 = {-0.5f, 0.5f, 0.0f};  // top left
    vec3 vertex2 = {0.5f, 0.5f, 0.0f};  // top right
    vec3 vertex3 = {0.5f, -0.5f, 0.0f};  // bottom right
    vec3 vertex4 = {-0.5f, -0.5f, 0.0f};  // bottom left

    bool useAtlas = false;
    int atlasNumRows = 0;
    int atlasRow = 0;
    int atlasColumn = 1;
    float atlasCellSize = 0.125f;

    void Start() override {
        vertex1.x = transform.pos.x - transform.width/2;
        vertex1.y = transform.pos.y + transform.height/2;
        vertex2.x = transform.pos.x + transform.width/2;
        vertex2.y = transform.pos.y + transform.height/2;
        vertex3.x = transform.pos.x + transform.width/2;
        vertex3.y = transform.pos.y - transform.height/2;
        vertex4.x = transform.pos.x - transform.width/2;
        vertex4.y = transform.pos.y - transform.height/2;
    }

    void Update() override {
        GameObject::Update();
    }
};

class Sound : public GameObject {
public:
    virtual ~Sound() = default;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
};

#endif //GAMEENGINE_GAMEOBJECT_H
