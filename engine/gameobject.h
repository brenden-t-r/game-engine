#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include "../constants.h"
#include "vector.h"

#include <cstdio>
#include <vector>

class Transform {
public:
    vector3 pos = {0, 0, 0};
    vector3 rot = {0, 0, 0};
    vector3 scale = {1, 1, 1};
    float width = 1;
    float height = 1;

    void Translate(vector3 vector){}
    void Rotate(float degrees){}
};

class Component {
public:
    virtual void Init() = 0;
    virtual void Update() = 0;
};

class GameObject {
public:
    Transform transform{};
    std::vector<Component*> components{};

    virtual void Update() {
        for (auto & component : components) {
            component->Update();
        }
    }
};

class Triangle : public GameObject {
public:
    vector3 vertex1 = {1.0f, -1.0f, 0.0f};
    vector3 vertex2 = {-1.0f, -1.0f, 0.0f};
    vector3 vertex3 = {0, 1.0f, 0.0f};

    void Update() override {
        GameObject::Update();
        vertex1.x = transform.pos.x + transform.width/2;
        vertex1.y = transform.pos.y - transform.height/2;
        vertex2.x = transform.pos.x - transform.width/2;
        vertex2.y = transform.pos.y - transform.height/2;
        vertex3.x = transform.pos.x;
        vertex3.y = transform.pos.y + transform.height/2;
    }
};

class Sprite : public GameObject {
public:
    vector3 vertex1 = {-0.5f, 0.5f, 0.0f};  // top left
    vector3 vertex2 = {0.5f, 0.5f, 0.0f};  // top right
    vector3 vertex3 = {0.5f, -0.5f, 0.0f};  // bottom right
    vector3 vertex4 = {-0.5f, -0.5f, 0.0f};  // bottom left

    bool useAtlas = false;
    int atlasNumRows = 0;
    int atlasRow = 0;
    int atlasColumn = 1;
    float atlasCellSize = 0.125f;

    void Update() override {
        GameObject::Update();
        vertex1.x = transform.pos.x - transform.width/2;
        vertex1.y = transform.pos.y + transform.height/2;
        vertex2.x = transform.pos.x + transform.width/2;
        vertex2.y = transform.pos.y + transform.height/2;
        vertex3.x = transform.pos.x + transform.width/2;
        vertex3.y = transform.pos.y - transform.height/2;
        vertex4.x = transform.pos.x - transform.width/2;
        vertex4.y = transform.pos.y - transform.height/2;
    }
};



#endif //GAMEENGINE_GAMEOBJECT_H
