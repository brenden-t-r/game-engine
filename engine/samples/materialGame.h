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

    Shader* LoadShader() {
        ShaderDef shaderDef{};
#if BACKEND_DIRECTX
        shaderDef.path = "assets/shaders/2colors.hlsl";
        shaderDef.inputLayoutType = InputLayoutType::POSITION;
#elif BACKEND_OPENGL
        shaderDef.vertexPath = "assets/shaders/color.glsl.vert";
        shaderDef.fragmentPath = "assets/shaders/2colors.glsl.frag";
#elif BACKEND_METAL
        shaderDef.vertexPath = "assets/shaders/color.vert.metal";
        shaderDef.fragmentPath = "assets/shaders/2colors.frag.metal";
#endif
        return platform->LoadShader(shaderDef);
    }

    Material* Get2ColorMaterial(Shader* shader) {
        auto m = new Material(shader);
        Material::UniformField field{};
        field.name = "Color1";
        field.type = Material::UniformFieldType::FLOAT4;
        field.f4[0] = 1.0f;
        field.f4[1] = 1.0f;
        field.f4[2] = 1.0f;
        field.f4[3] = 1.0f;
        m->uniformFields.push_back(field);
        Material::UniformField field2{};
        field2.name = "Color2";
        field2.type = Material::UniformFieldType::FLOAT4;
        field2.f4[0] = 1.0f;
        field2.f4[1] = 1.0f;
        field2.f4[2] = 1.0f;
        field2.f4[3] = 0.0f;
        m->uniformFields.push_back(field2);
        return m;
    }

    void Start() override {
        platform->LoadShaders();
        sprite = platform->CreateSprite("assets/sprites/background.png");
        texture = sprite->GetTexture();
        sprite->transform.width = 2.0;
        sprite->transform.height = 2.0;
        texture2 = platform->CreateTexture("assets/sprites/CardScarlet.png");
//        auto mat = (MaterialSprite*)sprite->material;
//        mat->color[3] = 0.8f;

        triangle = (Triangle*)platform->CreateTriangle();
        triangle->transform.width = 0.5;
        triangle->transform.height = 0.5;
        Shader* shader = LoadShader();
        Material* m = Get2ColorMaterial(shader);
        triangle->SetMaterial(m);
    }

    void Update() override {
        // Adjust constant buffer colors
        auto colorMaterial = (Material*)triangle->material;
        if (colorMaterial->uniformFields[0].f4[ind] > 0.9) {
            dir = -1;
        } else if (colorMaterial->uniformFields[0].f4[ind] < 0.1) {
            dir = 1;
            ind = ind == 2 ? 1 : 2;
        }
        colorMaterial->uniformFields[0].f4[ind] += 0.05f * dir;

        // Adjust sprite texture
//        auto spriteMaterial = (MaterialSprite*)sprite->material;
//        spriteMaterial->texture = dir == 1 ? texture : texture2;

        sprite->Update();
        triangle->Update();
    }

private:
    Triangle* triangle;
    Sprite* sprite;
    Texture* texture;
    Texture* texture2;
    float dir = 1;
    int ind = 2;
};

#endif //GAMEPROJECT_MATERIALGAME_H
