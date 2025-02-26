#ifndef GAMEENGINE_PONGGAME_H
#define GAMEENGINE_PONGGAME_H

#include <cassert>
#include "../constants.h"
#include "../engine/game.h"
#include "../engine/collision.h"
#include "../engine/gameobject.h"
#include "../platform/platform.h"

//#define debugging

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

        ballPos = vec3{0, -0.5, 0};
        ballDirVec = get_unit_vector_from_angle_degrees(45);
        ball = platform->CreateSprite("assets/sprites/ball.png");
        ball->transform.width = BallWidth;
        ball->transform.height = BallHeight;
        ball->transform.pos.x = ballPos.x;
        ball->transform.pos.y = ballPos.y;

        srand(101); // NOLINT(*-msc51-cpp); fixed seed for consistency in sample
    }

    void Update() override {
        player1Paddle->Update();
        player2Paddle->Update();
        ball->Update();

        // Ball movement
        {
            float speed = isQuick ? ballSpeed * 1.5f : ballSpeed;
            ballPos.x += speed * ballDirVec.x;
            ballPos.y += speed * ballDirVec.y;
            ball->transform.pos = ballPos;
        }

        // Paddle left keyboard movement
        {
            if (platform->IsKeyPressed(KeyCode::S)) {
                if (player1Paddle->transform.pos.y >= (-1.0f + PaddleHeight / 2)) {
                    player1Paddle->transform.pos.y -= 0.01f;
                }
            }
            if (platform->IsKeyPressed(KeyCode::W)) {
                if (player1Paddle->transform.pos.y <= (1.0f - PaddleHeight / 2)) {
                    player1Paddle->transform.pos.y += 0.01f;
                }
            }
            if (platform->IsKeyPressed(KeyCode::Down)) {
                if (player2Paddle->transform.pos.y >= (-1.0f + PaddleHeight / 2)) {
                    player2Paddle->transform.pos.y -= 0.01f;
                }
            }
            if (platform->IsKeyPressed(KeyCode::Up)) {
                if (player2Paddle->transform.pos.y <= (1.0f - PaddleHeight / 2)) {
                    player2Paddle->transform.pos.y += 0.01f;
                }
            }
        }

        // Collision of ball with left and right paddles
        {
            if (AABB_collision(ball, player1Paddle)) {
                if (platform->IsKeyPressed(KeyCode::W)) {
                    isQuick = ballDirVec.y >= 0;
                } else if (platform->IsKeyPressed(KeyCode::S)) {
                    isQuick = ballDirVec.y < 0;
                } else {
                    isQuick = false;
                }

                float stutter = getRandomFloat(1 - stutterAmt, 1 + stutterAmt);
                ballDirVec.x *= -1 * stutter;
                if (ballDirVec.x > 1) ballDirVec.x = 1;
                if (ballDirVec.x < -1) ballDirVec.x = -1;
                if (ballDirVec.y >= 0) {
                    ballDirVec.y = sqrtf(1 - powf(ballDirVec.x, 2));
                } else {
                    ballDirVec.y = -sqrtf(1 - powf(ballDirVec.x, 2));
                }
                if (ballPos.x <= (-1 + PaddleWidth)) {
                    ballPos.x = -1 + PaddleWidth + BallWidth;
                }
            }
            if (AABB_collision(ball, player2Paddle)) {
                if (platform->IsKeyPressed(KeyCode::Down)) {
                    isQuick = ballDirVec.y < 0;
                } else if (platform->IsKeyPressed(KeyCode::Up)) {
                    isQuick = ballDirVec.y >= 0;
                } else {
                    isQuick = false;
                }

                float stutter = getRandomFloat(1 - stutterAmt, 1 + stutterAmt);
                ballDirVec.x *= -1 * stutter;
                if (ballDirVec.x > 1) ballDirVec.x = 1;
                if (ballDirVec.x < -1) ballDirVec.x = -1;
                if (ballDirVec.y >= 0) {
                    ballDirVec.y = sqrtf(1 - powf(ballDirVec.x, 2));
                } else {
                    ballDirVec.y = -sqrtf(1 - powf(ballDirVec.x, 2));
                }
                if (ballPos.x > (1 - PaddleWidth - BallWidth)) {
                    ballPos.x = 1 - PaddleWidth - BallWidth;
                }
            }
        }

        // Collision of ball with top and bottom boundaries
        {
            if (ballPos.y > (1 - BallHeight / 2) || ballPos.y < (-1 + BallHeight)) {
                ballDirVec.y *= -1;
            }
        }

        // Game end checks
        {
            if (ballPos.x > 1.0) {
                printf("Player 1 wins");
                exit(0);
            }
            if (ballPos.x < -1.0) {
                printf("Player 2 wins");
                exit(0);
            }
        }

#ifdef debugging
        if (platform->IsMouseReleased(MouseButton::Right)) {
            stutterAmt += 0.05;
        }
        if (ballPos.x > (1 - PaddleWidth - BallWidth) || ballPos.x < (-1 + PaddleWidth)) {
            ballDirVec.x *= -1;
        }
#endif
    }

private:
    //region Private Variables
    Sprite* player1Paddle{};
    Sprite* player2Paddle{};
    Sprite* ball{};

    float ballSpeed = 0.01;
    float stutterAmt = 0.2;
    vec2 ballDirVec = vec2{};
    vec3 ballPos{};
    bool isQuick;

    static constexpr float PaddlePixelWidth = 32.0 * 2;
    static constexpr float PaddlePixelHeight = 128.0 * 2;
    static constexpr float BallPixelDiameter = 32.0 * 2;
    static constexpr float BallWidth = BallPixelDiameter/WINDOW_WIDTH;
    static constexpr float BallHeight = BallPixelDiameter/WINDOW_HEIGHT;
    static constexpr float PaddleWidth = PaddlePixelWidth/WINDOW_WIDTH;
    static constexpr float PaddleHeight = PaddlePixelHeight/WINDOW_HEIGHT;
    // endregion
};

#endif //GAMEENGINE_PONGGAME_H
