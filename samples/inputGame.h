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
        triangleCursor = platform->CreateTriangle(0, 0, 0.07, 0.07);
        triangleKeyUp = platform->CreateTriangle(0.2, 0.2, 0.09, 0.09);
        triangleKeyDown = platform->CreateTriangle(0, 0, 0.05, 0.05);

        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
        this->EnableCallback(Callback::GAMEPAD_RELEASED);
    }

    void KeyReleasedCallback(KeyCode key) override {
        if (key == KeyCode::W) {
            printf("W pressed impl\n");
        } else if (key == KeyCode::S) {
            printf("S pressed impl\n");
        }
        triangleKeyUp->transform.pos.x -= 0.01;
        shouldShowCallbackTriangle = true;

//        if (key == KeyCode::Up) {
//            vibrationSpeedL += 10000;
//        }
//        if (key == KeyCode::Down) {
//            vibrationSpeedL -= 10000;
//        }
//        if (key == KeyCode::Left) {
//            vibrationSpeedR += 10000;
//        }
//        if (key == KeyCode::Right) {
//            vibrationSpeedR -= 10000;
//        }
//        platform->SetGamepadVibration(vibrationSpeedL, vibrationSpeedR);

    }
    void MouseReleasedCallback(MouseButton button) override {
        triangleKeyUp->transform.pos.y -= 0.02;
        shouldShowCallbackTriangle = true;
    }
    void GamepadReleasedCallback(GamepadButton button) override {
        triangleKeyUp->transform.pos.y -= 0.02;
        shouldShowCallbackTriangle = true;
    }

    void Update() override {
        printf(".");

        if (shouldShowCallbackTriangle) {
            triangleKeyUp->Update();
        }
        shouldShowCallbackTriangle = false;

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
        if (platform->IsKeyPressed(KeyCode::W)) {
            triangleKeyDown->transform.pos.y += 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::S)) {
            triangleKeyDown->transform.pos.y -= 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::A)) {
            triangleKeyDown->transform.pos.x -= 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::D)) {
            triangleKeyDown->transform.pos.x += 0.01;
        }
        if (platform->IsGamepadButtonPressed(GamepadButton::DUp)) {
            triangleKeyDown->transform.pos.y += 0.01;
        }
        if (platform->IsGamepadButtonPressed(GamepadButton::DDown)) {
            triangleKeyDown->transform.pos.y -= 0.01;
        }
        if (platform->IsGamepadButtonPressed(GamepadButton::DLeft)) {
            triangleKeyDown->transform.pos.x -= 0.01;
        }
        if (platform->IsGamepadButtonPressed(GamepadButton::DRight)) {
            triangleKeyDown->transform.pos.x += 0.01;
        }
        triangleKeyDown->Update();

        if (platform->IsGamepadButtonPressed(GamepadButton::South)) {
            if (vibrationSpeedL <= 64000) vibrationSpeedL += 1000;
            printf("%d\n", vibrationSpeedL);
            platform->SetGamepadVibration(vibrationSpeedL, vibrationSpeedR);
        }
        if (platform->IsGamepadButtonPressed(GamepadButton::East)) {
            if (vibrationSpeedL > 0) vibrationSpeedL -= 1000;
            if (vibrationSpeedL < 0) vibrationSpeedL = 0;
            platform->SetGamepadVibration(vibrationSpeedL, vibrationSpeedR);
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
    GameObject* triangleCursor;
    GameObject* triangleKeyUp;
    GameObject* triangleKeyDown;
    bool shouldShowCallbackTriangle = false;
    int vibrationSpeedL = 0;
    int vibrationSpeedR = 0;
};

#endif //GAMEENGINE_INPUTGAME_H
