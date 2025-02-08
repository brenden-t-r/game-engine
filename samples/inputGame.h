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
        triangleL->transform.width = 0.05;
        triangleL->transform.height = 0.05;
        triangleL->transform.pos.x = -0.5;
        triangleL->transform.pos.y = 0.5;
        triangleR = platform->CreateTriangle();
        triangleR->transform.width = 0.05;
        triangleR->transform.height = 0.05;
        triangleR->transform.pos.x = 0;
        triangleR->transform.pos.y = 0.5;
        triangleM = platform->CreateTriangle();
        triangleM->transform.width = 0.05;
        triangleM->transform.height = 0.05;
        triangleM->transform.pos.x = 0.5;
        triangleM->transform.pos.y = 0.5;
        triangleL2 = platform->CreateTriangle();
        triangleL2->transform.width = 0.05;
        triangleL2->transform.height = 0.05;
        triangleL2->transform.pos.x = -0.5;
        triangleL2->transform.pos.y = -0.5;
        triangleR2 = platform->CreateTriangle();
        triangleR2->transform.width = 0.05;
        triangleR2->transform.height = 0.05;
        triangleR2->transform.pos.x = 0;
        triangleR2->transform.pos.y = -0.5;
        triangleM2 = platform->CreateTriangle();
        triangleM2->transform.width = 0.05;
        triangleM2->transform.height = 0.05;
        triangleM2->transform.pos.x = 0.5;
        triangleM2->transform.pos.y = -0.5;
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
    }

private:
    GameObject* triangleL;
    GameObject* triangleR;
    GameObject* triangleM;
    GameObject* triangleL2;
    GameObject* triangleR2;
    GameObject* triangleM2;
};

#endif //GAMEENGINE_INPUTGAME_H
