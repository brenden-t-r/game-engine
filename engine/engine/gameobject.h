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
        vertices[0].x = -transform.width/2;
        vertices[0].y = +transform.height/2;
        vertices[1].x = +transform.width/2;
        vertices[1].y = +transform.height/2;
        vertices[2].x = +transform.width/2;
        vertices[2].y = -transform.height/2;
        vertices[3].x = -transform.width/2;
        vertices[3].y = -transform.height/2;

        // Convert to screen coordinates since transformation matrix will be in screen coordinates
        vertices[0] = normalized_to_screen(vertices[0]);
        vertices[1] = normalized_to_screen(vertices[1]);
        vertices[2] = normalized_to_screen(vertices[2]);
        vertices[3] = normalized_to_screen(vertices[3]);

        // In normalized coordinates (0,0) is center of screen; in screen coordinates (0,0) is top-left.
        // Rotation happens about the origin, and our transformation matrix is in screen coordinates.
        vec3 screenOriginAdjustment = normalized_to_screen({ 0, 0, 0 });
        vertices[0] = {vertices[0].x - screenOriginAdjustment.x, vertices[0].y - screenOriginAdjustment.y, 1};
        vertices[1] = {vertices[1].x - screenOriginAdjustment.x, vertices[1].y - screenOriginAdjustment.y, 1};
        vertices[2] = {vertices[2].x - screenOriginAdjustment.x, vertices[2].y - screenOriginAdjustment.y, 1};
        vertices[3] = {vertices[3].x - screenOriginAdjustment.x, vertices[3].y - screenOriginAdjustment.y, 1};

        // Create transformation matrix
        vec3 translationPixels = normalized_to_screen(absolutePosition);
        Matrix3 transformation = matrix_transformation(translationPixels, absoluteScale, absoluteRot);

        // Apply transformation matrix to vertices in screen space
        vertices[0] = matrix_multiply_vec(transformation, vertices[0]);
        vertices[1] = matrix_multiply_vec(transformation, vertices[1]);
        vertices[2] = matrix_multiply_vec(transformation, vertices[2]);
        vertices[3] = matrix_multiply_vec(transformation, vertices[3]);

        // Convert vertices back to normalized coordinates
        vertices[0] = screen_to_normalized(vertices[0]);
        vertices[1] = screen_to_normalized(vertices[1]);
        vertices[2] = screen_to_normalized(vertices[2]);
        vertices[3] = screen_to_normalized(vertices[3]);
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
