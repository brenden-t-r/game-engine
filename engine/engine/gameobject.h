#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include "../constants.h"
#include "vector.h"
#include "matrix.h"

#include <cstdio>
#include <vector>
#include <cassert>

class Transform {
public:
    vec3 pos = {0, 0, 0};
    vec3 rot = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    float width = 1;
    float height = 1;

    Transform* parent;
};

class GameObject {
public:
    Transform transform{};
    virtual ~GameObject()= default;
    virtual void Update(){}
};

class Triangle : public GameObject {
public:
    vec3 vertices[3] {
        {1.0f, -1.0f, 0.0f},
        {-1.0f, -1.0f, 0.0f},
        {0, 1.0f, 0.0f},
    };

    Triangle() {
        SetPosition({0,0,0});
    }

    void Update() override {
        GameObject::Update();
        SetPosition(transform.pos);
    }

    void SetPosition(vec3 position) {
//        transform.pos = position;
//        vertices[0].x = transform.width/2;
//        vertices[0].y = -transform.height/2;
//        vertices[1].x = -transform.width/2;
//        vertices[1].y = -transform.height/2;
//        vertices[2].x = 0;
//        vertices[2].y = transform.height/2;
//        vertices[0] = scale_vector(vertices[0], transform.scale);
//        vertices[1] = scale_vector(vertices[1], transform.scale);
//        vertices[2] = scale_vector(vertices[2], transform.scale);
//        translate_vertices({0,0,0}, vertices, 3, transform.pos);
//        if (transform.rot.z != 0) {
//            rotate_vertices(vertices, 3, transform.pos, transform.rot.z);
//        }
    }

    void SetScale(vec3 newScale) {
        transform.scale = newScale;
    }

    void Translate(vec3 translate) {
        transform.pos.x += translate.x;
        transform.pos.y += translate.y;
        transform.pos.z += translate.z;
    }

    void Rotate(float degrees) {
        transform.rot.z += degrees;
    }
};

class Sprite : public GameObject {
public:
    vec3 vertices[4] {
            {-0.5f, 0.5f, 1.0f},
            {0.5f, 0.5f, 1.0f},
            {0.5f, -0.5f, 1.0f},
            {-0.5f, -0.5f, 1.0f},
    };

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
        SetPosition(transform.pos);
    }

    void SetPosition(vec3 position) {
        transform.pos = position;

        // Calculate local and absolute position, scale and rotation including inherited transforms
        vec3 localPosition = position;
        vec3 absolutePosition = localPosition;
        vec3 localScale = transform.scale;
        vec3 absoluteScale = localScale;
        vec3 localRot = transform.rot;
        vec3 absoluteRot = localRot;
        Transform* parent = transform.parent;
        while(parent != nullptr) {
            absoluteScale.x *= parent->scale.x;
            absoluteScale.y *= parent->scale.y;
            absoluteScale.z *= parent->scale.z;
            absoluteRot.x += parent->rot.x;
            absoluteRot.y += parent->rot.y;
            absoluteRot.z += parent->rot.z;
            absolutePosition = local_to_world(localPosition, parent->pos, absoluteScale, absoluteRot.z);
            parent = parent->parent;
        }

        // Start the object with the appropriate width and height at the origin in normalized coordinates
        vertices[0] = {-transform.width/2, +transform.height/2, 1};
        vertices[1] = {+transform.width/2, +transform.height/2, 1};
        vertices[2] = {+transform.width/2, -transform.height/2, 1};
        vertices[3] = {-transform.width/2, -transform.height/2, 1};

        // Create transformation matrix
        Matrix3 transformation = matrix_transformation(absolutePosition, absoluteScale, absoluteRot);

        // Apply transformation matrix
        for (auto & vertice : vertices) {
            vertice = matrix_multiply_vec(transformation, vertice);
        }
    }

    void SetScale(vec3 newScale) {
        transform.scale = newScale;
    }

    void Translate(vec3 translate) {
        transform.pos.x += translate.x;
        transform.pos.y += translate.y;
        transform.pos.z += translate.z;
    }

    void Rotate(float degrees) {
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
