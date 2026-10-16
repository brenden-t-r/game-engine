#pragma once
#define DEBUG_QUAD 0

#include "engine/game.h"
#include "../engine/platform/directx/directx.h"
#include "model.h"
#include "util.h"

class IsoGame : public Game {
public:
    using Game::Game;

    void Start() override {
        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
        platform->LoadShaders();

        auto shader = platform->LoadShader({
            "assets/shaders/color.hlsl","","",InputLayoutType::POSITION
        });
        mat = new MaterialColor(shader);

        quad = getQuad(platform, {-0.5, -0.5}, {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5});
    }

    MaterialColor* mat = nullptr;
    Quad* quad = nullptr;

    void Update() override {
        mat->color[0]=0.0;
        mat->color[1]=0.0;
        mat->color[2]=1.0;
        mat->color[3]=1.0;
        quad->SetMaterial(mat);
        quad->Update();

        SetColor(mat->color, Color{0.0,0.0,0.0,10/255.0});
        drawIsoGrid(platform, mat, 0);

        SetColor(mat->color, COLOR::BLACK);
    }

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        if (key == MouseButton::Left) {
            printf("mouse click\n");
        }
    }
    void KeyReleasedCallback(KeyCode key) override {
        printf("%d\n",key);
        switch (key) {
            case KeyCode::R:
                break;
            case KeyCode::E:
                break;
            case KeyCode::_1:
                break;
            case KeyCode::_2:
                break;
            case KeyCode::_3:
                break;
            case KeyCode::_4:
                break;
            case KeyCode::_5:
                break;
            case KeyCode::L:
                break;
            case KeyCode::W:
                break;
            case KeyCode::H:
                break;
            case KeyCode::S:
                break;
            case KeyCode::Up:
                break;
            case KeyCode::Down:
                break;
            default:
                break;
        }
    }
    void Delete(Quad* o) {
        platform->Delete(o->a);
        platform->Delete(o->b);
        delete o;
    }
};