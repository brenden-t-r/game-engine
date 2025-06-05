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
    float _11, _12, _13 = 0;
    float _21, _22, _23 = 0;
    float _31, _32, _33 = 0;

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
};

Matrix3 matrix_multiply(Matrix3 a, Matrix3 b) {
    return {
            dot_product(a.Row(0), b.Col(0)), dot_product(a.Row(0), b.Col(1)), dot_product(a.Row(0), b.Col(2)),
            dot_product(a.Row(1), b.Col(0)), dot_product(a.Row(1), b.Col(1)), dot_product(a.Row(1), b.Col(2)),
            dot_product(a.Row(2), b.Col(0)), dot_product(a.Row(2), b.Col(1)), dot_product(a.Row(2), b.Col(2))
    };
}

//void MatrixTransformation(Transform t) {
//    Matrix3 matrix = {
//            0, 0, 0,
//            0, 0, 0,
//            0, 0, 0,
//    };
//    matrix = {
//            t.scale.x
//    };
//
//    float angle = t.rot.z;
//    float cos = cosf(angle);
//    float sin = sinf(angle);
//    Matrix3 rotationMatrix = {
//            cos, -sin, 0,
//            sin, cos,  0,
//            0,   0,    0,
//    };
//
//    //?
//    Matrix3 scaleMatrix = {
//            t.scale.x, 0, 0,
//            0, t.scale.y, 0,
//            0, 0, 0
//    };
//}

class Sprite : public GameObject {
public:
    vec3 vertices[4] {
            {-0.5f, 0.5f, 0.0f},
            {0.5f, 0.5f, 0.0f},
            {0.5f, -0.5f, 0.0f},
            {-0.5f, -0.5f, 0.0f},
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
        vertices[0].x = -transform.width/2;
        vertices[0].y = +transform.height/2;
        vertices[1].x = +transform.width/2;
        vertices[1].y = +transform.height/2;
        vertices[2].x = +transform.width/2;
        vertices[2].y = -transform.height/2;
        vertices[3].x = -transform.width/2;
        vertices[3].y = -transform.height/2;
        vertices[0] = scale_vector(vertices[0], transform.scale);
        vertices[1] = scale_vector(vertices[1], transform.scale);
        vertices[2] = scale_vector(vertices[2], transform.scale);
        vertices[3] = scale_vector(vertices[3], transform.scale);
        translate_vertices(vertices, 4, transform.pos);
        if (transform.rot.z != 0) {
            rotate_vertices(vertices, 4, transform.pos, transform.rot.z);
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
