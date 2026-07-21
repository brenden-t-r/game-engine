#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include "../constants.h"
#include "vector.h"
#include "matrix.h"
#include "texture.h"
#include "material.h"

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

static Matrix3 to_transform_matrix(Transform* transform) {
    return matrix_transformation(transform->pos, transform->scale, transform->rot);
}
static vec3 local_to_world_conversion(Transform* transform, vec3 point) {
    vec3 posScreen = normalized_to_screen(transform->pos);
    Matrix3 M = local_to_world_matrix(posScreen, transform->scale, transform->rot);
    vec3 worldPoint = matrix_multiply_vec(M, point);
    return screen_to_normalized(worldPoint);
}
static vec3 world_to_local_conversion(Transform* transform, vec3 point) {
    vec3 posScreen = normalized_to_screen(transform->pos);
    Matrix3 M = local_to_world_matrix(posScreen, transform->scale, transform->rot);
    Matrix3 M_I = matrix_inverse(M);
    vec3 worldPoint = matrix_multiply_vec(M_I, point);
    return screen_to_normalized(worldPoint);
}

class EngineComponent {
public:
    virtual ~EngineComponent() = default;
    virtual void Update() = 0;
    Transform* transform;
};

class GameObject {
public:
    Transform transform{};
    Material* material;
    std::vector<EngineComponent*> components{};
    virtual ~GameObject() {
        for (auto & component : components) {
            delete component;
        }
    };
    virtual void SetMaterial(Material* mat) {
        material = mat;
    }
    virtual void Update(){
        for (auto & component : components) {
            component->Update();
        }
    }
    void AddComponent(EngineComponent* component) {
        component->transform = &transform;
        components.push_back(component);
    }
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
        SetPosition(transform.pos);
        GameObject::Update();
    }

    void SetPosition(vec3 position) {
        transform.pos = position;

        // Start the object with the appropriate width and height at the origin in normalized coordinates
        vertices[0] = {transform.width/2, -transform.height/2, 1};
        vertices[1] = {-transform.width/2, -transform.height/2, 1};
        vertices[2] = {0, transform.height/2, 1};

        // Create transformation matrix
        Matrix3 localMatrix = to_transform_matrix(&transform);
        Matrix3 transformation = localMatrix;

        // Multiply by inherited transforms to get final transform matrix
        Transform* parent = transform.parent;
        while (parent != nullptr) {
            Matrix3 parentMatrix = to_transform_matrix(parent);
            transformation = parentMatrix.multiply(transformation);
            parent = parent->parent;
        }

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
struct Glyph {
    int x, y, w, h;
    int xoff, yoff;
    float advance;
};
#include "text_msdf.h"
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

    bool useGlyph = false;
    float glyphX = 0;
    float glyphY = 0;
    float glyphW = 0;
    float glyphH = 0;
    float glyphxoff = 0;
    float glyphyoff = 0;
    float atlasWidth = 0;
    float atlasHeight = 0;

    Sprite() {
        SetPosition({0,0,0});
    }

    virtual Texture* GetTexture() = 0;

    void Update() override {
        SetPosition(transform.pos);
        GameObject::Update();
    }

    void SetPosition(vec3 position) {
        transform.pos = position;

        // Start the object with the appropriate width and height at the origin in normalized coordinates
        if (useAtlas && useGlyph) {
            float ndcXoff = glyphxoff;
            float ndcYoff = glyphyoff;
            float w = (glyphW*2.0f/ (float)FRAMEBUFFER_WIDTH)  ;
            float h = (glyphH*2.0f / (float)FRAMEBUFFER_HEIGHT) ;
            float x0 = ndcXoff;
            float y0 = ndcYoff;
            float x1 = x0 + w;
            float y1 = y0 - h;   // minus because top-left to bottom-left in NDC
            vertices[0] = {x0, y0, 1};  // top-left
            vertices[1] = {x1, y0, 1};  // top-right
            vertices[2] = {x1, y1, 1};  // bottom-right
            vertices[3] = {x0, y1, 1};  // bottom-left
        } else {
            vertices[0] = {-transform.width/2, +transform.height/2, 1};
            vertices[1] = {+transform.width/2, +transform.height/2, 1};
            vertices[2] = {+transform.width/2, -transform.height/2, 1};
            vertices[3] = {-transform.width/2, -transform.height/2, 1};
        }

        // Create transformation matrix
        Matrix3 localMatrix = to_transform_matrix(&transform);
        Matrix3 transformation = localMatrix;

        // Multiply by inherited transforms to get final transform matrix
        Transform* parent = transform.parent;
        while (parent != nullptr) {
            Matrix3 parentMatrix = to_transform_matrix(parent);
            transformation = parentMatrix.multiply(transformation);
            parent = parent->parent;
        }

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
    ~Sound() override = default;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
};

#endif //GAMEENGINE_GAMEOBJECT_H
