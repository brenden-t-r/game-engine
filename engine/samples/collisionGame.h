#ifndef GAMEENGINE_COLLISIONGAME_H
#define GAMEENGINE_COLLISIONGAME_H

#define DEBUG_COLLIDERS

#include "../engine/game.h"
#include "../engine/collision.h"

class CollisionGame : public Game {
public:
    using Game::Game;

    ~CollisionGame() override {
        platform->Delete(objects[0]);
        platform->Delete(objects[1]);
        platform->Delete(objects[2]);
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
        objects[2]->Rotate(30);
        objects[1]->SetScale(vec3{0.5, 0.5, 1});
        objects[1]->Rotate(-45);
        EnableCallback(MOUSE_RELEASED);
    }

    void Update() override {
        objects[1]->Update();
        objects[2]->Update();

        // Check collision of two box colliders with no rotation
        int resultAABB = AABB_collision(objects[2], objects[1]);

        // Check collision of two object-oriented box colliders with rotation
        OOBB_box_collider a = get_OOBB_box_collider(objects[2]);
        OOBB_box_collider b = get_OOBB_box_collider(objects[1]);
        int resultOOBB = OOBB_collision(a, b);

        // Check collision of mouse position point against rotated box collider
        mousePos = platform->GetMousePos();
        int resultOOBBPoint = OOBB_collision(objects[2], mousePos);

#ifdef AABB
        int boxColliderResult = resultAABB;
#else
        int boxColliderResult = resultOOBB;
#ifdef DEBUG_COLLIDERS
        debug_OOBB_collider(platform, a);
        debug_OOBB_collider(platform, b);
#endif
#endif
        if (boxColliderResult || resultOOBBPoint) {
            objects[0]->Update();
        }

        // Movement
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
        if (platform->IsKeyPressed(KeyCode::Up)) {
            objects[1]->transform.rot.z += 1;
        }
        if (platform->IsKeyPressed(KeyCode::Down)) {
            objects[1]->transform.rot.z -= 1;
        }
    }

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        mousePos = platform->GetMousePos();
    }

private:
    Sprite* objects[3];
    float speed = 0.005;
    vec3 mousePos;
};

#endif //GAMEENGINE_COLLISIONGAME_H
