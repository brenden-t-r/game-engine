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
        objects[0] = platform->CreateSprite("assets/sprites/ball.png");
        objects[0]->transform.pos = vec3{0, 0.9, 0};
        objects[0]->transform.width = 0.1f;
        objects[0]->transform.height = 0.1f;
        objects[1] = platform->CreateSprite("assets/sprites/background.png");
        objects[1]->transform.pos = vec3{-0.3, 0, 0};
        objects[1]->transform.width = 0.25f;
        objects[1]->transform.height = 0.25f;
        objects[2] = platform->CreateSprite("assets/sprites/paddle.png");
        objects[2]->transform.pos = vec3{0, 0, 0};
        objects[2]->transform.width = 0.25f;
        objects[2]->transform.height = 0.25f;
    }

    void Update() override {
        objects[1]->Update();
        objects[2]->Update();

        int result = AABB_collision(objects[1], objects[2]);
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
    float speed = 0.005;
};

#endif //GAMEENGINE_COLLISIONGAME_H
