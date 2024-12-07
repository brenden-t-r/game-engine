#ifndef GAMEENGINE_VECTOR_H
#define GAMEENGINE_VECTOR_H

#include <cstdio>
#include <vector>

class Vector3 {
public:
    float x;
    float y;
    float z;
};

class Transform {
public:
    Vector3 pos = {0, 0, 0};
    Vector3 rot = {0, 0, 0};
    Vector3 scale = {1, 1, 1};
    float width = 1;
    float height = 1;

    void Translate(Vector3 vector){}
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

class SpriteRenderer : public Component {
    void Init() override{
        printf("SpriteRenderer::Init");
    }
    void Update() override{
        printf("SpriteRenderer::Update");
    }
};

class Triangle : public GameObject {
public:
    Vector3 vertex1 = { 0.5f,  -0.5f, 0.0f};
    Vector3 vertex2 = {-0.5f, -0.5f, 0.0f};
    Vector3 vertex3 = {0, 0.5f, 0.0f};

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



#endif //GAMEENGINE_VECTOR_H
