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
    virtual ~GameObject()= default;
    virtual void Update(){}
};

class Triangle : public GameObject {
public:
    vec3 vertex1 = {1.0f, -1.0f, 0.0f};
    vec3 vertex2 = {-1.0f, -1.0f, 0.0f};
    vec3 vertex3 = {0, 1.0f, 0.0f};

    Triangle() {
        SetPosition({0,0,0});
    }

    void Update() override {
        GameObject::Update();
    }

    void SetPosition(vec3 position) {
        transform.pos = position;
        vertex1.x = transform.pos.x + transform.width/2;
        vertex1.y = transform.pos.y - transform.height/2;
        vertex2.x = transform.pos.x - transform.width/2;
        vertex2.y = transform.pos.y - transform.height/2;
        vertex3.x = transform.pos.x;
        vertex3.y = transform.pos.y + transform.height/2;
        if (transform.rot.z > 0) {
            Rotate(transform.rot.z);
        }
    }

    void Translate(vec3 translate) {
        vertex1.x += translate.x;
        vertex2.x += translate.x;
        vertex3.x += translate.x;
        vertex1.y += translate.y;
        vertex2.y += translate.y;
        vertex3.y += translate.y;
        vertex1.z += translate.z;
        vertex2.z += translate.z;
        vertex3.z += translate.z;
        transform.pos.x += translate.x;
        transform.pos.y += translate.y;
        transform.pos.z += translate.z;
    }
    
    void Rotate(float degrees) {
        // "Undo" current position transform back to screen space origin (top left 0,0)
        vec3 posScreenSpace = coords_device_to_screen(transform.pos);

        vertex1.x = coords_screen_to_device({coords_device_to_screen(vertex1).x - posScreenSpace.x, coords_device_to_screen(vertex1).y - posScreenSpace.y, 0}).x;
        vertex2.x = coords_screen_to_device({coords_device_to_screen(vertex2).x - posScreenSpace.x, coords_device_to_screen(vertex2).y - posScreenSpace.y, 0}).x;
        vertex3.x = coords_screen_to_device({coords_device_to_screen(vertex3).x - posScreenSpace.x, coords_device_to_screen(vertex3).y - posScreenSpace.y, 0}).x;
        vertex1.y = coords_screen_to_device({coords_device_to_screen(vertex1).x - posScreenSpace.x, coords_device_to_screen(vertex1).y - posScreenSpace.y, 0}).y;
        vertex2.y = coords_screen_to_device({coords_device_to_screen(vertex2).x - posScreenSpace.x, coords_device_to_screen(vertex2).y - posScreenSpace.y, 0}).y;
        vertex3.y = coords_screen_to_device({coords_device_to_screen(vertex3).x - posScreenSpace.x, coords_device_to_screen(vertex3).y - posScreenSpace.y, 0}).y;

        vertex1 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex1), degrees));
        vertex2 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex2), degrees));
        vertex3 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex3), degrees));

        vertex1.x = coords_screen_to_device({coords_device_to_screen(vertex1).x + posScreenSpace.x, coords_device_to_screen(vertex1).y + posScreenSpace.y, 0}).x;
        vertex2.x = coords_screen_to_device({coords_device_to_screen(vertex2).x + posScreenSpace.x, coords_device_to_screen(vertex2).y + posScreenSpace.y, 0}).x;
        vertex3.x = coords_screen_to_device({coords_device_to_screen(vertex3).x + posScreenSpace.x, coords_device_to_screen(vertex3).y + posScreenSpace.y, 0}).x;
        vertex1.y = coords_screen_to_device({coords_device_to_screen(vertex1).x + posScreenSpace.x, coords_device_to_screen(vertex1).y + posScreenSpace.y, 0}).y;
        vertex2.y = coords_screen_to_device({coords_device_to_screen(vertex2).x + posScreenSpace.x, coords_device_to_screen(vertex2).y + posScreenSpace.y, 0}).y;
        vertex3.y = coords_screen_to_device({coords_device_to_screen(vertex3).x + posScreenSpace.x, coords_device_to_screen(vertex3).y + posScreenSpace.y, 0}).y;

        transform.rot.z += degrees;
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

    Sprite() {
        SetPosition({0,0,0});
    }

    void Update() override {
        GameObject::Update();
    }

    void SetPosition(vec3 position) {
        transform.pos = position;
        vertex1.x = transform.pos.x - transform.width/2;
        vertex1.y = transform.pos.y + transform.height/2;
        vertex2.x = transform.pos.x + transform.width/2;
        vertex2.y = transform.pos.y + transform.height/2;
        vertex3.x = transform.pos.x + transform.width/2;
        vertex3.y = transform.pos.y - transform.height/2;
        vertex4.x = transform.pos.x - transform.width/2;
        vertex4.y = transform.pos.y - transform.height/2;
        if (transform.rot.z > 0) {
            Rotate(transform.rot.z);
        }
    }

    void Translate(vec3 translate) {
        vertex1.x += translate.x;
        vertex2.x += translate.x;
        vertex3.x += translate.x;
        vertex4.x += translate.x;
        vertex1.y += translate.y;
        vertex2.y += translate.y;
        vertex3.y += translate.y;
        vertex4.y += translate.y;
        vertex1.z += translate.z;
        vertex2.z += translate.z;
        vertex3.z += translate.z;
        vertex4.z += translate.z;
        transform.pos.x += translate.x;
        transform.pos.y += translate.y;
        transform.pos.z += translate.z;
    }
    
    void Rotate(float degrees) {
        // "Undo" current position transform back to screen space origin (top left 0,0)
        vec3 posScreenSpace = coords_device_to_screen(transform.pos);

        vertex1.x = coords_screen_to_device({coords_device_to_screen(vertex1).x - posScreenSpace.x, coords_device_to_screen(vertex1).y - posScreenSpace.y, 0}).x;
        vertex2.x = coords_screen_to_device({coords_device_to_screen(vertex2).x - posScreenSpace.x, coords_device_to_screen(vertex2).y - posScreenSpace.y, 0}).x;
        vertex3.x = coords_screen_to_device({coords_device_to_screen(vertex3).x - posScreenSpace.x, coords_device_to_screen(vertex3).y - posScreenSpace.y, 0}).x;
        vertex4.x = coords_screen_to_device({coords_device_to_screen(vertex4).x - posScreenSpace.x, coords_device_to_screen(vertex4).y - posScreenSpace.y, 0}).x;
        vertex1.y = coords_screen_to_device({coords_device_to_screen(vertex1).x - posScreenSpace.x, coords_device_to_screen(vertex1).y - posScreenSpace.y, 0}).y;
        vertex2.y = coords_screen_to_device({coords_device_to_screen(vertex2).x - posScreenSpace.x, coords_device_to_screen(vertex2).y - posScreenSpace.y, 0}).y;
        vertex3.y = coords_screen_to_device({coords_device_to_screen(vertex3).x - posScreenSpace.x, coords_device_to_screen(vertex3).y - posScreenSpace.y, 0}).y;
        vertex4.y = coords_screen_to_device({coords_device_to_screen(vertex4).x - posScreenSpace.x, coords_device_to_screen(vertex4).y - posScreenSpace.y, 0}).y;

        vertex1 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex1), degrees));
        vertex2 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex2), degrees));
        vertex3 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex3), degrees));
        vertex4 = coords_screen_to_device(rotate_euler(coords_device_to_screen(vertex4), degrees));

        vertex1.x = coords_screen_to_device({coords_device_to_screen(vertex1).x + posScreenSpace.x, coords_device_to_screen(vertex1).y + posScreenSpace.y, 0}).x;
        vertex2.x = coords_screen_to_device({coords_device_to_screen(vertex2).x + posScreenSpace.x, coords_device_to_screen(vertex2).y + posScreenSpace.y, 0}).x;
        vertex3.x = coords_screen_to_device({coords_device_to_screen(vertex3).x + posScreenSpace.x, coords_device_to_screen(vertex3).y + posScreenSpace.y, 0}).x;
        vertex4.x = coords_screen_to_device({coords_device_to_screen(vertex4).x + posScreenSpace.x, coords_device_to_screen(vertex4).y + posScreenSpace.y, 0}).x;
        vertex1.y = coords_screen_to_device({coords_device_to_screen(vertex1).x + posScreenSpace.x, coords_device_to_screen(vertex1).y + posScreenSpace.y, 0}).y;
        vertex2.y = coords_screen_to_device({coords_device_to_screen(vertex2).x + posScreenSpace.x, coords_device_to_screen(vertex2).y + posScreenSpace.y, 0}).y;
        vertex3.y = coords_screen_to_device({coords_device_to_screen(vertex3).x + posScreenSpace.x, coords_device_to_screen(vertex3).y + posScreenSpace.y, 0}).y;
        vertex4.y = coords_screen_to_device({coords_device_to_screen(vertex4).x + posScreenSpace.x, coords_device_to_screen(vertex4).y + posScreenSpace.y, 0}).y;

        transform.rot.z += degrees;
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
