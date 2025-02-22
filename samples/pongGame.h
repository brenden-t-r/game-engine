#ifndef GAMEENGINE_PONGGAME_H
#define GAMEENGINE_PONGGAME_H

#include "../constants.h"
#include "../engine/game.h"
#include "../engine/vector.h"
#include "../platform/platform.h"

class PongGame : public Game {
public:
    using Game::Game;

    ~PongGame() override {
        delete player1Paddle;
        delete player2Paddle;
        delete ball;
    };

    void Start() override {
        platform->LoadShaders();
        player1Paddle = platform->CreateSprite("assets/sprites/paddle.png");
        player1Paddle->transform.width = PaddleWidth;
        player1Paddle->transform.height = PaddleHeight;
        player1Paddle->transform.pos.x = -1 + (PaddleWidth/2);
        player1Paddle->transform.pos.y = 1 - (PaddleHeight/2);

        player2Paddle = platform->CreateSprite("assets/sprites/paddle.png");
        player2Paddle->transform.width = PaddleWidth;
        player2Paddle->transform.height = PaddleHeight;
        player2Paddle->transform.pos.x = 1 - (PaddleWidth/2);
        player2Paddle->transform.pos.y = 1 - (PaddleHeight/2);

        ballPos = vector3{0, -0.5, 0};
        ballDir = vector3{1, 1, 0};
        ball = platform->CreateSprite("assets/sprites/ball.png");
        ball->transform.width = BallWidth;
        ball->transform.height = BallHeight;
        ball->transform.pos.x = ballPos.x;
        ball->transform.pos.y = ballPos.y;
    }

    void Update() override {
        printf(".");
        player1Paddle->Update();
        player2Paddle->Update();
        ball->Update();

        // Paddle left keyboard movement
        if (platform->IsKeyPressed(KeyCode::S)) {
            if (player1Paddle->transform.pos.y >= (-1.0f + PaddleHeight/2)) {
                player1Paddle->transform.pos.y -= 0.01f;
            }
        }
        if (platform->IsKeyPressed(KeyCode::W)) {
            if (player1Paddle->transform.pos.y <= (1.0f - PaddleHeight/2)) {
                player1Paddle->transform.pos.y += 0.01f;
            }
        }

        // Ball movement
        ballPos.x += ballSpeed * ballDir.x;
        ballPos.y += ballSpeed * ballDir.y;
        ball->transform.pos = ballPos;

        if (ballPos.x > (1 - PaddleWidth - BallWidth) || ballPos.x < (-1 + PaddleWidth)) {
            ballDir.x *= -1;
        }
        if (ballPos.y > (1 - BallHeight/2) || ballPos.y < (-1 + BallHeight)) {
            ballDir.y *= -1;
        }
    }

private:
    Sprite* player1Paddle{};
    Sprite* player2Paddle{};
    Sprite* ball{};

    float temp = 0;
    float tempDir = -1;
    float ballSpeed = 0.01f;
    vector3 ballPos{};
    vector3 ballDir{};

    static constexpr float PaddlePixelWidth = 32.0 * 2;
    static constexpr float PaddlePixelHeight = 128.0 * 2;
    static constexpr float BallPixelDiameter = 32.0 * 2;
    static constexpr float BallWidth = BallPixelDiameter/WINDOW_WIDTH;
    static constexpr float BallHeight = BallPixelDiameter/WINDOW_HEIGHT;
    static constexpr float PaddleWidth = PaddlePixelWidth/WINDOW_WIDTH;
    static constexpr float PaddleHeight = PaddlePixelHeight/WINDOW_HEIGHT;
};

#endif //GAMEENGINE_PONGGAME_H
