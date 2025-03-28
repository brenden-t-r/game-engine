#if 0
#ifndef GAMEENGINE_AUDIOGAME_H
#define GAMEENGINE_AUDIOGAME_H

#include "../engine/game.h"
#include "../engine/audio.h"
#include "cstdio"

constexpr int BAR_COUNT = 16;

struct Beat {
    bool kick;
    bool snare;
    bool hat;
    bool ride;
    bool crash;
};

struct Bar {
    Beat beats[4];
};

struct Song {
    Bar bars[BAR_COUNT];
};

static Song MySong = {
    {
        // Section 1
        {
                {
                        { true, false, false, true, true },
                        { false, false, true, false },
                        { false, true, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, false, false, true },
                        { false, false, true, false },
                        { false, true, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, false, false, true },
                        { false, false, true, false },
                        { false, true, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, false, false, true },
                        { false, false, true, false },
                        { false, true, true, false },
                        { false, false, true, false }
                }
        },
        // Section 2
        {
                {
                        { true, false, false, true },
                        { true, false, false, true },
                        { false, true, false, true },
                        { false, true, false, true }
                }
        },
        {
                {
                        { true, false, false, true },
                        { true, false, false, true },
                        { false, true, false, true },
                        { false, true, false, true }
                }
        },
        {
                {
                        { true, false, false, true },
                        { true, false, false, true },
                        { false, true, false, true },
                        { false, true, false, true }
                }
        },
        {
                {
                        { true, true, false, false },
                        { true, true, false, false },
                        { true, true, false, false },
                        { true, true, false, false }
                }
        },
        // Section 3
        {
                {
                        { true, false, false, true, true },
                        { false, false, true, false },
                        { false, false, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, true, false, true },
                        { false, false, false, false },
                        { true, true, false, true },
                        { false, false, false, false }

                }
        },
        {
                {
                        { true, false, false, true },
                        { false, false, true, false },
                        { false, false, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, false, false, false },
                        { false, true, false, false },
                        { false, false, true, false },
                        { false, false, false, true }

                }
        },
        // Section 4
        {
                {
                        { true, false, false, true, true },
                        { false, false, true, false },
                        { false, false, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, true, false, true },
                        { false, false, false, false },
                        { true, true, false, true },
                        { false, false, false, false }

                }
        },
        {
                {
                        { true, false, false, true },
                        { false, false, true, false },
                        { false, false, true, false },
                        { false, false, true, false }
                }
        },
        {
                {
                        { true, true, false, true, true },
                        { false, false, false, true, true },
                        { true, true, false, true, true },
                        { false, false, false, true, true }

                }
        },
    }
};

class AudioGame : public Game {
public:
    using Game::Game;
    ~AudioGame() override {
        delete kick;
        delete snare;
        delete hat;
        delete triangle;
        delete ride;
        delete crash;
        delete audioWrapper;
    };

    void Start() override {
        audioWrapper = new AudioWrapper();
        AudioWrapper::Init();

        kick = new AudioSoundWrapper();
        kick->Init("assets/audio/Kick.mp3");
        snare = new AudioSoundWrapper();
        snare->Init("assets/audio/Snare.mp3");
        hat = new AudioSoundWrapper();
        hat->Init("assets/audio/Hat.mp3");
        ride = new AudioSoundWrapper();
        ride->Init("assets/audio/Ride.mp3");
        crash = new AudioSoundWrapper();
        crash->Init("assets/audio/Crash.mp3");
        ma_sound_set_volume(&hat->sound, 0.5f);
        ma_sound_set_volume(&crash->sound, 0.5f);

        platform->LoadShaders();
        triangle = platform->CreateTriangle();
    }

    int speed = 12;
    int bars = -1;
    int beats = 0;
    int count = 0;
    void Update() override {
        printf(".");
        triangle->Update();

        count++;
        if (count % speed == 0) {
//            kick->Reset();
//            kick->Play();

            if (beats % 4 == 0 ) {
                bars++;
                beats = 0;
//                ride->Reset();
//                ride->Play();
            }

            if (bars >= BAR_COUNT) {
                bars = 0;
            }
            auto beat = MySong.bars[bars].beats[beats];

            if (beat.kick) {
                kick->Reset();
                kick->Play();
            }
            if (beat.snare) {
                snare->Reset();
                snare->Play();
            }
            if (beat.ride) {
                ride->Reset();
                ride->Play();
            }
            if (beat.hat) {
                hat->Reset();
                hat->Play();
            }
            if (beat.crash) {
                crash->Reset();
                crash->Play();
            }

            beats++;
        }
    }

private:
    GameObject* triangle = nullptr;
    AudioWrapper* audioWrapper = nullptr;
    AudioSoundWrapper* kick = nullptr;
    AudioSoundWrapper* snare = nullptr;
    AudioSoundWrapper* hat = nullptr;
    AudioSoundWrapper* ride = nullptr;
    AudioSoundWrapper* crash = nullptr;
};

#endif //GAMEENGINE_AUDIOGAME_H
#endif