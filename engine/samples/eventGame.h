#ifndef GAMEPROJECT_EVENTGAME_H
#define GAMEPROJECT_EVENTGAME_H

#include "../engine/game.h"

typedef void (*EventCallback);
typedef void (*EventCallbackWithParam)(void*);
typedef void (*EventCallbackWithTwoParam)(void*, void*);

template<typename T>
class Event {
public:
    typedef void (*EventCallback)(T*);

    Event() : _count(0) {}

    void subscribe(EventCallback cb) {
        if (_count < 16) {
            _callbacks[_count++] = cb;
        }
    }

    void unsubscribe(EventCallback cb) {
        for (int i = 0; i < _count; ++i) {
            if (_callbacks[i] == cb) {
                _callbacks[i] = _callbacks[_count - 1];
                --_count;
                return;
            }
        }
    }

    void publish(T* param) {
        for (int i = 0; i < _count; ++i) {
            _callbacks[i](param);
        }
    }

private:
    EventCallback _callbacks[16];
    int _count;
};

class Event1 {
public:
    void subscribe(EventCallbackWithParam func) {
        if (_count < 16) {
            _callbacks[_count] = func;
            ++_count;
        }
    }

    void unsubscribe(EventCallbackWithParam cb) {
        for (int i = 0; i < _count; ++i) {
            if (_callbacks[i] == cb) {
                // Shift the rest down
                for (int j = i; j < _count - 1; ++j) {
                    _callbacks[j] = _callbacks[j + 1];
                }
                --_count;
                return;
            }
        }
    }

    void publish(void* param) {
        for (int i = 0; i < _count; ++i) {
            _callbacks[i](param);
        }
    }

private:
    EventCallbackWithParam _callbacks[16]{};
    void* _userDatas[16]{};
    int _count = 0;
};

class EventGame : public Game {
public:
    using Game::Game;

    void Start() override {
        platform->LoadShaders();
        triangle = platform->CreateTriangle();
        MyEvent.subscribe(OnEventCallback);
        MyEvent1.subscribe(OnEventCallback);
    }

    void Update() override {
        triangle->Update();
        if (platform->IsKeyPressed(KeyCode::D)) {
            MyEvent.publish(this);
        }
    }

    static void OnEventCallback(void* _this) {
        printf("hey there");
        ((EventGame*)_this);
    }

    static void OnEventCallback(EventGame* _this) {
        printf("hey there");
        _this->triangle->transform.pos.x += 0.05;
    }

private:
    GameObject* triangle;
    Event<EventGame> MyEvent;
    Event1 MyEvent1;
};



#endif //GAMEPROJECT_EVENTGAME_H
