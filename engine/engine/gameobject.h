#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include "../constants.h"
#include "vector.h"

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

struct Matrix3 {
    float _11 = 0, _12 = 0, _13 = 0;
    float _21 = 0, _22 = 0, _23 = 0;
    float _31 = 0, _32 = 0, _33 = 0;

    vec3 Row(int i) {
        switch (i) {
            case 0:
                return vec3{_11, _12, _13};
            case 1:
                return vec3{_21, _22, _23};
            case 2:
                return vec3{_31, _32, _33};
            default:
                assert(false);
        }
    }
    vec3 Col(int i) {
        switch (i) {
            case 0:
                return vec3{_11, _21, _31};
            case 1:
                return vec3{_12, _22, _32};
            case 2:
                return vec3{_13, _23, _33};
            default:
                assert(false);
        }
    }

    Matrix3 multiply(Matrix3 b) {
        return {
                dot(Row(0), b.Col(0)), dot(Row(0), b.Col(1)), dot(Row(0), b.Col(2)),
                dot(Row(1), b.Col(0)), dot(Row(1), b.Col(1)), dot(Row(1), b.Col(2)),
                dot(Row(2), b.Col(0)), dot(Row(2), b.Col(1)), dot(Row(2), b.Col(2))
        };
    }
};

Matrix3 matrix_multiply(Matrix3 a, Matrix3 b) {
    return a.multiply(b);
}

vec3 matrix_multiply_vec(Matrix3 mat, vec3 vec) {
    return {
        dot(mat.Row(0), vec), // (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
        dot(mat.Row(1), vec),
        dot(mat.Row(2), vec)
    };
}
//vec3 matrix_multiply_vec(Matrix3 mat, vec3 vec) {
//    return {
//            dot(vec, mat.Col(0)),
//            dot(vec, mat.Col(1)),
//            dot(vec, mat.Col(2))
//    };
//}

Matrix3 matrix_transformation(vec3 position, vec3 scale, vec3 rotation) {
    // Rotation matrix
    float angle = rotation.z * PI/180.0f;
    float cos = cosf(angle);
    float sin = sinf(angle);
    Matrix3 R = {
            cos, -sin, 0,
            sin, cos,  0,
            0,   0,    1,
    };

    // Scale matrix
    Matrix3 S = {
            scale.x, 0,       0,
            0,       scale.y, 0,
            0,       0,       1
    };

    // Translation
    Matrix3 T = {
            1, 0, position.x,
            0, 1, position.y,
            0, 0, 1
    };

    // Scale -> Rotate -> Translate
    return T.multiply(R).multiply(S);
    /*
     *   scale.x * cos, -sin,          position.x,
     *   sin,         , scale.y * cos, position.y,
     *   0,           , 0            , 1
     */
}

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

        vec3 localPosition = position;
        vec3 absolutePosition = localPosition;
        vec3 localScale = transform.scale;
        vec3 absoluteScale = localScale;
        vec3 localRot = transform.rot;
        vec3 absoluteRot = localRot;

        if (transform.parent != nullptr) {
            absoluteScale.x *= transform.parent->scale.x;
            absoluteScale.y *= transform.parent->scale.y;
            absoluteScale.z *= transform.parent->scale.z;

            absoluteRot.x += transform.parent->rot.x;
            absoluteRot.y += transform.parent->rot.y;
            absoluteRot.z += transform.parent->rot.z;

            vec3 scaledLocalPosition = scale_vector(localPosition, transform.parent->scale);
            vec3 rotatedLocalPosition = rotate_euler(scaledLocalPosition, transform.parent->rot.z);
            absolutePosition.x = transform.parent->pos.x + rotatedLocalPosition.x;
            absolutePosition.y = transform.parent->pos.y + rotatedLocalPosition.y;
            absolutePosition.z = transform.parent->pos.z + rotatedLocalPosition.z;

            absolutePosition = local_to_world(localPosition, transform.parent->pos, absoluteScale, -absoluteRot.z);
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

//        vertices[0] = scale_vector(vertices[0], transform.scale);
//        vertices[1] = scale_vector(vertices[1], transform.scale);
//        vertices[2] = scale_vector(vertices[2], transform.scale);
//        vertices[3] = scale_vector(vertices[3], transform.scale);
//        translate_vertices(vertices, 4, transform.pos);
//        if (transform.rot.z != 0) {
//            rotate_vertices(vertices, 4, transform.pos, transform.rot.z);
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

class Sound : public GameObject {
public:
    virtual ~Sound() = default;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
};

#endif //GAMEENGINE_GAMEOBJECT_H
