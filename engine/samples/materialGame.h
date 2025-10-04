#ifndef GAMEPROJECT_MATERIALGAME_H
#define GAMEPROJECT_MATERIALGAME_H

#include "../engine/game.h"
#include "../engine/material.h"
#include "../engine/texture.h"

class MaterialGame : public Game {
public:
    using Game::Game;

    ~MaterialGame() override {
    };

    void Start() override {
        platform->LoadShaders();

        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.5;
        triangle->transform.height = 0.5;
        auto colorMaterial = (MaterialColor*)triangle->material;
        colorMaterial->color[0] = 0.3;
        colorMaterial->color[1] = 0.5;
        colorMaterial->color[2] = 0.7;
        colorMaterial->color[3] = 1.0;

        sprite = platform->CreateSprite("assets/sprites/background.png");
        sprite->transform.width = 2.0;
        sprite->transform.height = 2.0;
        auto spriteMaterial = (MaterialSprite*)sprite->material;
    }

    void Update() override {
        sprite->Update();
        triangle->Update();
        auto colorMaterial = (MaterialColor*)triangle->material;
        if (colorMaterial->color[0] < 1.0) {
            colorMaterial->color[0] += 0.01;
        } else {
            colorMaterial->color[0] -= 0.01;
        }
    }

private:
    Triangle* triangle;
    Sprite* sprite;
};

#endif //GAMEPROJECT_MATERIALGAME_H
