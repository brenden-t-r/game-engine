#ifndef GAMEENGINE_INPUTGAME_H
#define GAMEENGINE_INPUTGAME_H

#include "../platform/platform.h"
#include "../engine/game.h"

class InputGame : public Game {
public:
    using Game::Game;

    ~InputGame() override = default;

    void Start() override {
        platform->LoadShaders();
        triangleL = platform->CreateTriangle();
        triangleL->transform.width = 0.05f;
        triangleL->transform.height = 0.05f;
        triangleL->transform.pos.x = -0.5f;
        triangleL->transform.pos.y = 0.5f;
        triangleR = platform->CreateTriangle();
        triangleR->transform.width = 0.05f;
        triangleR->transform.height = 0.05f;
        triangleR->transform.pos.x = 0;
        triangleR->transform.pos.y = 0.5f;
        triangleM = platform->CreateTriangle();
        triangleM->transform.width = 0.05f;
        triangleM->transform.height = 0.05f;
        triangleM->transform.pos.x = 0.5f;
        triangleM->transform.pos.y = 0.5f;
        triangleL2 = platform->CreateTriangle();
        triangleL2->transform.width = 0.05f;
        triangleL2->transform.height = 0.05f;
        triangleL2->transform.pos.x = -0.5f;
        triangleL2->transform.pos.y = -0.5f;
        triangleR2 = platform->CreateTriangle();
        triangleR2->transform.width = 0.05f;
        triangleR2->transform.height = 0.05f;
        triangleR2->transform.pos.x = 0;
        triangleR2->transform.pos.y = -0.5f;
        triangleM2 = platform->CreateTriangle();
        triangleM2->transform.width = 0.05f;
        triangleM2->transform.height = 0.05f;
        triangleM2->transform.pos.x = 0.5f;
        triangleM2->transform.pos.y = -0.5f;
        triangleCursor = platform->CreateTriangle();
        triangleCursor->transform.width = 0.07f;
        triangleCursor->transform.height = 0.07f;
        triangleCursor->transform.pos.x = 0;
        triangleCursor->transform.pos.y = 0;
        triangleKeyUp = platform->CreateTriangle();
        triangleKeyUp->transform.width = 0.07f;
        triangleKeyUp->transform.height = 0.07f;
        triangleKeyUp->transform.pos.x = 0.9;
        triangleKeyUp->transform.pos.y = 0.9;
    }

    void Update() override {
        printf(".");

        if (platform->IsMousePressed(MouseButton::Left)) {
            printf("\nLeft Pressed!");
            triangleL->Update();
        }
        if (platform->IsMousePressed(MouseButton::Right)) {
            printf("\nRight Pressed!");
            triangleR->Update();
        }
        if (platform->IsMousePressed(MouseButton::Middle)) {
            printf("\nMiddle Pressed!");
            triangleM->Update();
        }

        if (platform->IsMouseReleased(MouseButton::Left)) {
            printf("\nLeft Released!");
            triangleL2->Update();
        }
        if (platform->IsMouseReleased(MouseButton::Right)) {
            printf("\nRight Released!");
            triangleR2->Update();
        }
        if (platform->IsMouseReleased(MouseButton::Middle)) {
            printf("\nMiddle Released!");
            triangleM2->Update();
        }

        if (platform->IsKeyReleased(KeyCode::W)) {
            triangleKeyUp->Update();
        }

        auto pos = platform->GetMousePos();
        triangleCursor->transform.pos.x = pos.x;
        triangleCursor->transform.pos.y = pos.y;
        triangleCursor->Update();
    }

private:
    GameObject* triangleL;
    GameObject* triangleR;
    GameObject* triangleM;
    GameObject* triangleL2;
    GameObject* triangleR2;
    GameObject* triangleM2;
    GameObject* triangleCursor;
    GameObject* triangleKeyUp;
};

#endif //GAMEENGINE_INPUTGAME_H
