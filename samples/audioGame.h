#ifndef GAMEENGINE_AUDIOGAME_H
#define GAMEENGINE_AUDIOGAME_H

#include "../engine/game.h"
#include "../engine/audio.h"
#include "cstdio"

class AudioGame : public Game {
public:
    using Game::Game;
    ~AudioGame() override {
        delete kick;
        delete snare;
        delete hat;
        delete triangle;
        delete ride;
        delete audioWrapper;
    };

    void Start() override {
        audioWrapper = new AudioWrapper();
        audioWrapper->Init();
        //audioWrapper->Play(R"(C:\Users\Brenden\Desktop\2025Song9.mp3)");

        kick = new AudioSoundWrapper();
        kick->Init(R"(C:\Users\Brenden\Desktop\Kick.wav)");
        snare = new AudioSoundWrapper();
        snare->Init(R"(C:\Users\Brenden\Desktop\Snare.wav)");
        hat = new AudioSoundWrapper();
        hat->Init(R"(C:\Users\Brenden\Desktop\Hat.wav)");
        ride = new AudioSoundWrapper();
        ride->Init(R"(C:\Users\Brenden\Desktop\Ride.wav)");
        ma_sound_set_volume(&hat->sound, 0.5f);

        platform->LoadShaders();
        triangle = platform->CreateTriangle();
    }

    int countKick = 0;
    int countSnare = 0;
    int countHat = 0;
    int countRide = 0;
    int speed = 12;
    int bars = 1;
    int barCount = 0;
    void Update() override {
        printf(".");
        triangle->Update();

        barCount++;
        if (barCount == speed*4) {
            barCount = 0;
            bars++;
        }
        if (bars == 1) {
            //bars = 0;
        }

        if (bars <= 16) {
            if (countHat == speed*2 || countHat == 0) {
                countHat = 0;
                hat->Reset();
                hat->Play();
            }
        } else {
            if (countHat >= speed || countHat == 0) {
                countHat = 0;
                hat->Reset();
                hat->Play();
            }
        }

        if (countKick == speed*4|| countKick == 0) {
            countKick = 0;
            kick->Reset();
            kick->Play();
        }
        if (countSnare == speed*4) {
            countSnare = -speed*4;
            snare->Reset();
            snare->Play();
        }
        if (countRide == speed*32 || countRide == 0) {
            countRide = 0;
            ride->Reset();
            ride->Play();
        }
        countKick++;
        countSnare++;
        countHat++;
        countRide++;
    }

private:
    GameObject* triangle = nullptr;
    AudioWrapper* audioWrapper = nullptr;
    AudioSoundWrapper* kick = nullptr;
    AudioSoundWrapper* snare = nullptr;
    AudioSoundWrapper* hat = nullptr;
    AudioSoundWrapper* ride = nullptr;
};

#endif //GAMEENGINE_AUDIOGAME_H
