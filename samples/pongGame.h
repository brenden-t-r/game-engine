#ifndef GAMEENGINE_PONGGAME_H
#define GAMEENGINE_PONGGAME_H

#include <cassert>
#include "../constants.h"
#include "../engine/game.h"
#include "../engine/collision.h"
#include "../engine/gameobject.h"
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

        tests_getUnitVectorFromAngleDegrees();
        srand(101);

        ballDirVec = vector2{0.707,0.707};
    }

    bool isQuick = false;
    vector2 ballDirVec = vector2{0.707,0.707};

    vector2 ballDir75 = vector2{};

    /*
     *      15()
     *    /|
     * 1 / |
     *  /__|
     * 75() 90()
     *
     *
     * 1 = x^2 + y^2
     *
     * law of cosines
     * y = sqrt(x^2 + 1^2 - 2x(1)cos75)
     * y = sqrt(x^2 + 1 - 2xcos75)
     * y = sqrt(0.268^2 + 1 - 2(0.268)cos75)
     * y = sqrt(0.933124) = 0.966
     *
     * law of sines
     * x = c sin(a)/sin(b)
     * x = 1 sin(15)/sin(90) = 0.2588
     *
     */

    static vector2 getUnitVectorFromAngleDegrees(float angle) {
        float radiansConv = (3.14159265358979323846f / 180.0f);
        float radians = angle * radiansConv;
        float radians90 = 90 * radiansConv;
        float x = 1 * (float)sinf(radians90-radians) / (float)sinf(radians90);
        float y = sqrtf(powf(x, 2) + 1 - 2*x*1*cosf(radians));
        return vector2{
            x, y
        };
    }

    static void tests_getUnitVectorFromAngleDegrees() {
        vector2 vec = getUnitVectorFromAngleDegrees(45);
        assert(fabsf(vec.x - 0.707f) < 0.001f);
        assert(fabsf(vec.y - 0.707f) < 0.001f);
        vec = getUnitVectorFromAngleDegrees(75);
        assert(fabsf(vec.x - 0.259f) < 0.001f);
        assert(fabsf(vec.y - 0.966f) < 0.001f);
    }

    static int getRandomInt(int start, int end) {
        return start + (rand() % (end - start + 1));
    }
    static float getRandomFloat(float start, float end) {
        return start + static_cast<float>(rand()) / RAND_MAX * (end - start);
    }
    float stutterAmt = 0.2;

#define debuggin

    void Update() override {
//        printf(".");
        player1Paddle->Update();
        player2Paddle->Update();
        ball->Update();


        // Ball movement
        float speed = isQuick ? ballSpeed * 1.5f : ballSpeed;

        ballPos.x += speed * ballDirVec.x;
        ballPos.y += speed * ballDirVec.y;

//        ballPos.x += speed * ballDir.x;
//        ballPos.y += speed * ballDir.y;
        ball->transform.pos = ballPos;

#if 0
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
        if (platform->IsKeyPressed(KeyCode::Down)) {
            if (player2Paddle->transform.pos.y >= (-1.0f + PaddleHeight/2)) {
                player2Paddle->transform.pos.y -= 0.01f;
            }
        }
        if (platform->IsKeyPressed(KeyCode::Up)) {
            if (player2Paddle->transform.pos.y <= (1.0f - PaddleHeight/2)) {
                player2Paddle->transform.pos.y += 0.01f;
            }
        }

        if (AABB_collision(ball, player1Paddle)) {
            if (platform->IsKeyPressed(KeyCode::W)) {
                isQuick = ballDir.y == 1;
            }
            if (platform->IsKeyPressed(KeyCode::S)) {
                isQuick = ballDir.y == -1;
            } else {
                isQuick = false;
            }
            ballDir.x *= -1;

            if (ballPos.x < (-1 + PaddleWidth)) {
                ballPos.x = -1 + PaddleWidth;
            }
        }
        if (AABB_collision(ball, player2Paddle)) {
            if (platform->IsKeyPressed(KeyCode::Down)) {
                isQuick = ballDir.y == 1;
            }
            if (platform->IsKeyPressed(KeyCode::Up)) {
                isQuick = ballDir.y == -1;
            } else {
                isQuick = false;
            }
            ballDir.x *= -1;
            if (ballPos.x > (1 - PaddleWidth - BallWidth)) {
                ballPos.x = 1-PaddleWidth-BallWidth;
            }
        }

        // Ball movement
        float speed = isQuick ? ballSpeed * 1.5f : ballSpeed;
        ballPos.x += speed * ballDir.x;
        ballPos.y += speed * ballDir.y;
        ball->transform.pos = ballPos;

        if (ballPos.x > 1.0) {
            printf("Player 1 wins");
            exit(0);
        }
        if (ballPos.x < -1.0) {
            printf("Player 2 wins");
            exit(0);
        }
#endif

        if (platform->IsMouseReleased(MouseButton::Right)) {
            stutterAmt += 0.05;
        }
        float stutter = getRandomFloat(1-stutterAmt, 1+stutterAmt);

#ifdef debuggin
        if (ballPos.x > (1 - PaddleWidth - BallWidth) || ballPos.x < (-1 + PaddleWidth)) {
            ballDir.x *= -1;
            printf("%f\n", ballDirVec.x);
            ballDirVec.x *= -1 * stutter;

            if (ballDirVec.x > 1) ballDirVec.x = 1;
            if (ballDirVec.x < -1) ballDirVec.x = -1;
            if (ballDirVec.y < -1) ballDirVec.y = -1;

            if (ballDirVec.y >= 0) {
                ballDirVec.y = sqrtf(1 - powf(ballDirVec.x,2));
            } else {
                ballDirVec.y = -sqrtf(1 - powf(ballDirVec.x,2));
            }

//            ballDirVec.y = 1 - ballDirVec.x;

            printf("%f,%f\n", ballDirVec.x,ballDirVec.y);
        }
#endif
        if (ballPos.y > (1 - BallHeight/2) || ballPos.y < (-1 + BallHeight)) {
            ballDir.y *= -1;
            ballDirVec.y *= -1;// * stutter;
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
