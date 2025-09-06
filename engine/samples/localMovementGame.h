#ifndef GAMEPROJECT_LOCALMOVEMENTGAME_H
#define GAMEPROJECT_LOCALMOVEMENTGAME_H

#include "../engine/game.h"

class LocalMovementGame : public Game {
public:
    using Game::Game;

    ~LocalMovementGame() override = default;

    void Start() override {
        platform->LoadShaders();
        object = platform->CreateTriangle();
        object->transform.width = 0.2f;
        object->transform.height = 0.2f;
        o2 = platform->CreateTriangle();
        o2->transform.width = 0.02f;
        o2->transform.height = 0.02f;
        nested = platform->CreateTriangle();
        nested->transform.width = 0.03f;
        nested->transform.height = 0.03f;
        nested->transform.parent = &object->transform;
        nested->transform.pos.y = 0.3f;
    }

    void Update() override {
        o2->transform.pos = local_to_world_conversion(&object->transform, {0, -60, 1});
        o2->transform.rot.z = object->transform.rot.z;

        if (platform->IsKeyPressed(KeyCode::W)) {
            object->transform.pos = local_to_world_conversion(&object->transform, {0, -2, 1});
        }
        if (platform->IsKeyPressed(KeyCode::A)) {
            object->transform.pos.x -= 0.003;
        }
        if (platform->IsKeyPressed(KeyCode::D)) {
            object->transform.pos.x += 0.003;
        }
        if (platform->IsKeyPressed(KeyCode::Right)) {
            object->transform.rot.z += 1;
        }
        if (platform->IsKeyPressed(KeyCode::Left)) {
            object->transform.rot.z -= 1;
        }
        if (platform->IsKeyPressed(KeyCode::Up)) {
            object->transform.scale.x += 0.01;
            object->transform.scale.y += 0.01;
        }

        object->Update();
        o2->Update();
        nested->Update();
    }

    GameObject* object;
    GameObject* o2;
    GameObject* nested;
};

#endif //GAMEPROJECT_LOCALMOVEMENTGAME_H
