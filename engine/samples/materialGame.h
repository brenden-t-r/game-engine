#ifndef GAMEPROJECT_MATERIALGAME_H
#define GAMEPROJECT_MATERIALGAME_H

#include "../engine/game.h"
#include "../engine/material.h"
#include "../engine/texture.h"

class MaterialGame : public Game {
public:
    using Game::Game;

    ~MaterialGame() override {};

    void Start() override {

        Shader* colorShader = platform->LoadShader("assets/shaders/SimpleShader.hlsl", InputLayoutType::POSITION);
        Shader* textureShader = platform->LoadShader("assets/shaders/TextureShader.hlsl", InputLayoutType::POSITION_TEXCOORD);

        MaterialColor* colorMaterial = new MaterialColor(colorShader);
        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.5;
        triangle->transform.height = 0.5;
        colorMaterial->color[0] = 0.3;
        colorMaterial->color[1] = 0.5;
        colorMaterial->color[2] = 0.7;
        colorMaterial->color[3] = 1.0;
        triangle->AddComponent(new MaterialComponent(colorMaterial));

        sprite = platform->CreateSprite("assets/sprites/background.png");
        sprite->transform.width = 2.0;
        sprite->transform.height = 2.0;
        MaterialSprite* spriteMaterial = new MaterialSprite(textureShader, sprite->GetTexture());
        sprite->AddComponent(new MaterialComponent(spriteMaterial));
    }

    void Update() override {
        sprite->Update();
//        triangle->Update();
    }

private:
    Triangle* triangle;
    Sprite* sprite;
};

#endif //GAMEPROJECT_MATERIALGAME_H
