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

//        triangle = (Triangle*)platform->CreateTriangle();
//        triangle->transform.width = 0.5;
//        triangle->transform.height = 0.5;
//        auto colorMaterial = (MaterialTwoColors*)triangle->material;
//        colorMaterial->color1[0] = 1.0;
//        colorMaterial->color1[1] = 0.0;
//        colorMaterial->color1[2] = 0.0;
//        colorMaterial->color1[3] = 1.0;
//        colorMaterial->color2[0] = 0.0;
//        colorMaterial->color2[1] = 1.0;
//        colorMaterial->color2[2] = 0.0;
//        colorMaterial->color2[3] = 1.0;

//        sprite = platform->CreateSprite("assets/sprites/background.png");
//        sprite->transform.width = 2.0;
//        sprite->transform.height = 2.0;
//        auto spriteMaterial = (MaterialSprite*)sprite->material;

        auto* m = new MaterialWithUniformBuffer(nullptr);
        MaterialWithUniformBuffer::UniformField field{};
        field.name = "Color1";
        field.type = MaterialWithUniformBuffer::UniformFieldType::FLOAT4;
        field.f4[0] = 1.0f;
        field.f4[1] = 0.0f;
        field.f4[2] = 0.0f;
        field.f4[3] = 1.0f;
        m->fields.push_back(field);
        MaterialWithUniformBuffer::UniformField field2{};
        field.name = "Color2";
        field.type = MaterialWithUniformBuffer::UniformFieldType::FLOAT4;
        field.f4[0] = 0.0f;
        field.f4[1] = 1.0f;
        field.f4[2] = 0.0f;
        field.f4[3] = 1.0f;
        m->fields.push_back(field2);

    }

    float dir = 1;
    int ind = 3;
    void Update() override {
        sprite->Update();
//        auto colorMaterial = (MaterialColor*)triangle->material;
//
//        if (colorMaterial->color[ind] > 0.9) {
//            dir = -1;
//        } else if (colorMaterial->color[ind] < 0.1) {
//            dir = 1;
//        }
//        colorMaterial->color[ind] += 0.03f * dir;

        triangle->Update();
    }

private:
    Triangle* triangle;
    Sprite* sprite;
};

#endif //GAMEPROJECT_MATERIALGAME_H
