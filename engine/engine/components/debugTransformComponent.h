#ifndef GAMEPROJECT_DEBUGTRANSFORMCOMPONENT_H
#define GAMEPROJECT_DEBUGTRANSFORMCOMPONENT_H

#include "../gameobject.h"
#include "../../platform/platform.h"

enum class CoordinateOrientation {
    World = 0,
    Local = 1
};

class DebugTransformComponent : public EngineComponent {
public:
    explicit DebugTransformComponent(Platform* platform, CoordinateOrientation orientation)
            : platform(platform), orientation(orientation) {}

    void Update() override {
        if (platform->IsKeyPressed(KeyCode::W)) {
            if (orientation == CoordinateOrientation::Local) {
                transform->pos = local_to_world_conversion(transform, {0, -2, 1});
            } else {
                transform->pos.y += 0.003;
            }
        }
        if (platform->IsKeyPressed(KeyCode::S)) {
            if (orientation == CoordinateOrientation::Local) {
                transform->pos = local_to_world_conversion(transform, {0, 2, 1});
            } else {
                transform->pos.y -= 0.003;
            }
        }
        if (platform->IsKeyPressed(KeyCode::A)) {
            transform->pos.x -= 0.003;
        }
        if (platform->IsKeyPressed(KeyCode::D)) {
            transform->pos.x += 0.003;
        }
        if (platform->IsKeyPressed(KeyCode::Right)) {
            transform->rot.z += 1;
        }
        if (platform->IsKeyPressed(KeyCode::Left)) {
            transform->rot.z -= 1;
        }
        if (platform->IsKeyPressed(KeyCode::Up)) {
            transform->scale.x += 0.01;
            transform->scale.y += 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::Down)) {
            transform->scale.x -= 0.01;
            transform->scale.y -= 0.01;
        }
    }

private:
    Platform* platform;
    CoordinateOrientation orientation;
};

#endif //GAMEPROJECT_DEBUGTRANSFORMCOMPONENT_H
