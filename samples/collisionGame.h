#ifndef GAMEENGINE_COLLISIONGAME_H
#define GAMEENGINE_COLLISIONGAME_H

#include "../engine/game.h"
#include "../engine/collision.h"

class CollisionGame : public Game {
public:
    using Game::Game;

    ~CollisionGame() override {
        delete objects[0];
        delete objects[1];
        delete objects[2];
    }

    void Start() override {
        platform->LoadShaders();
        objects[0] = platform->CreateSprite("assets/sprites/paddle.png");
        objects[0]->transform.pos = Vector3{0,0.9,0};
        objects[0]->transform.width = 0.1f;
        objects[0]->transform.height = 0.1f;
        objects[1] = platform->CreateSprite("assets/sprites/background.png");
        objects[1]->transform.pos = Vector3{-0.3,0,0};
        objects[1]->transform.width = 0.25f;
        objects[1]->transform.height = 0.25f;
        objects[2] = platform->CreateSprite("assets/sprites/paddle.png");
        objects[2]->transform.pos = Vector3{0,0,0};
        objects[2]->transform.width = 0.25f;
        objects[2]->transform.height = 0.25f;
    }

    float speed = 0.005;
    void Update() override {
        objects[1]->Update();
        objects[2]->Update();

        auto obj1_pos = vector2{ objects[1]->transform.pos.x, objects[1]->transform.pos.y };
        auto obj1_collider = box_collider{
                {0, 0},
                objects[1]->transform.width,
                objects[1]->transform.height,
                PIVOT::CENTER
        };
        auto obj2_pos = vector2{ objects[2]->transform.pos.x, objects[2]->transform.pos.y };
        auto obj2_collider = box_collider{
                {0, 0},
                objects[2]->transform.width,
                objects[2]->transform.height,
                PIVOT::CENTER
        };

        int result = AABB_collision(obj1_pos, obj2_pos, obj1_collider, obj2_collider);
        if (result) {
            objects[0]->Update();
        }

        if (platform->IsKeyPressed(KeyCode::A)) {
            objects[1]->transform.pos.x -= speed;
        }
        if (platform->IsKeyPressed(KeyCode::D)) {
            objects[1]->transform.pos.x += speed;
        }
        if (platform->IsKeyPressed(KeyCode::W)) {
            objects[1]->transform.pos.y += speed;
        }
        if (platform->IsKeyPressed(KeyCode::S)) {
            objects[1]->transform.pos.y -= speed;
        }
    }

private:
    Sprite* objects[3];
};

#endif //GAMEENGINE_COLLISIONGAME_H
