#include "engine/game.h"
#include "../engine/platform/directx/directx.h"

class Quad {
public:
    Triangle* a = nullptr;
    Triangle* b = nullptr;
    void Update() {
        a->Update();
        b->Update();
    }
};

class IsoGame : public Game {
public:
    using Game::Game;

    MaterialColor* mat = nullptr;
    void Start() override {
        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
        platform->LoadShaders();

        auto shader = platform->LoadShader({
            "assets/shaders/color.hlsl","","",InputLayoutType::POSITION
        });
        mat = new MaterialColor(shader);
        mat->color[0]=0.0;
        mat->color[1]=0.0;
        mat->color[2]=1.0;
        mat->color[3]=1.0;

    }

    void Update() override {
        mat->color[0]=0.0;
        mat->color[1]=0.0;
        mat->color[2]=1.0;
        mat->color[3]=1.0;
        ((PlatformDirectX*)platform)->drawLine(vec2{-1,1}, vec2{0,0}, mat);
        mat->color[0]=1.0;
        mat->color[1]=0.0;
        mat->color[2]=0.0;
        mat->color[3]=1.0;
        ((PlatformDirectX*)platform)->drawLine(vec2{0,0}, vec2{1,1}, mat);
    }

    Triangle* getTriangle(vec2 _1, vec2 _2, vec2 _3) {
        auto t = (Triangle*)platform->CreateTriangle();
        t->vertices[0] = {_1.x, _1.y, 0.0};
        t->vertices[1] = {_2.x, _2.y, 0.0};
        t->vertices[2] = {_3.x, _3.y, 0.0};
        return t;
    }
    Quad* getQuad(vec2 _1, vec2 _2, vec2 _3, vec2 _4) {
        auto o = new Quad();
        o->a = getTriangle(_1, _2, _3);
        o->b = getTriangle(_2, _3, _4);
        o->b = new Triangle();
        return o;
    }
    void Delete(Quad* o) {
        platform->Delete(o->a);
        platform->Delete(o->b);
        delete o;
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

};