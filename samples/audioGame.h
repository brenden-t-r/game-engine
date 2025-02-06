#ifndef GAMEENGINE_AUDIOGAME_H
#define GAMEENGINE_AUDIOGAME_H

#include "../engine/game.h"
#include "../engine/audio.h"
#include "cstdio"

class AudioGame : public Game {
public:
    using Game::Game;
    ~AudioGame() override {
        delete audioSound;
        delete triangle;
        delete audioWrapper;
    };

    void Start() override {
        audioWrapper = new AudioWrapper();
        audioWrapper->Init();
        //audioWrapper->Play(R"(C:\Users\Brenden\Desktop\2025Song9.mp3)");

        audioSound = new AudioSoundWrapper();
        audioSound->Init(R"(C:\Users\Brenden\Desktop\Kick.wav)");
        audioSound->Play();

        platform->LoadShaders();
        triangle = platform->CreateTriangle();
    }

    int count = 0;
    void Update() override {
        printf(".");
        triangle->Update();

        count+=1;
        if (count == 25) {
            count = 0;
            audioSound->Reset();
            audioSound->Play();
        }
    }

private:
    GameObject* triangle = nullptr;
    AudioWrapper* audioWrapper = nullptr;
    AudioSoundWrapper* audioSound = nullptr;
};

#endif //GAMEENGINE_AUDIOGAME_H
