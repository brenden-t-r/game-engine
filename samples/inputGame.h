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
        triangleL = platform->CreateTriangle(-0.5f, 0.5f, 0.05, 0.05);
        triangleR = platform->CreateTriangle(0, 0.5f, 0.05, 0.05);
        triangleM = platform->CreateTriangle(0.5f, 0.5f, 0.05, 0.05);
        triangleL2 = platform->CreateTriangle(-0.5f, -0.5f, 0.05, 0.05);
        triangleR2 = platform->CreateTriangle(0, -0.5, 0.05, 0.05);
        triangleM2 = platform->CreateTriangle(0.5, -0.5, 0.05, 0.05);
        triangleCursor = platform->CreateTriangle(0, 0, 0.07, 0.07);
        triangleKeyUp = platform->CreateTriangle(0.2, 0.2, 0.09, 0.09);

        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
    }

    bool shouldShowKeyReleasedTriangle = false;
    void KeyReleasedCallback(KeyCode key) override {
        if (key == KeyCode::W) {
            printf("W pressed impl\n");
        } else if (key == KeyCode::S) {
            printf("S pressed impl\n");
        }
        triangleKeyUp->transform.pos.x -= 0.01;
        shouldShowKeyReleasedTriangle = true;
    }
    void MouseReleasedCallback(MouseButton button) override {
        triangleKeyUp->transform.pos.y -= 0.02;
        shouldShowKeyReleasedTriangle = true;
    }

    void Update() override {
        printf(".");

        if (shouldShowKeyReleasedTriangle) {
            triangleKeyUp->Update();
        }
        shouldShowKeyReleasedTriangle = false;

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
