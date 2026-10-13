#include "engine/game.h"

class IsoGame : public Game {
public:
    using Game::Game;

    void Start() override {
        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
        platform->LoadShaders();
    }

    void Update() override {

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