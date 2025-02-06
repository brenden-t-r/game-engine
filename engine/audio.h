#define MINIAUDIO_IMPLEMENTATION
#include "../dependencies/miniaudio.h"

static ma_engine g_engine;

class AudioWrapper {
public:
    ~AudioWrapper(){
        ma_engine_uninit(&g_engine);
    };

    void Init() {
        ma_result result;
        result = ma_engine_init(nullptr, &g_engine);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio engine.");
        }
        assert(result == MA_SUCCESS);
    }

    void Play(const char* filePath) {
        ma_engine_play_sound(&g_engine, filePath, nullptr);
    }
};

class AudioSoundWrapper {
public:
    ~AudioSoundWrapper(){
        ma_sound_uninit(&sound);
    };

    void Init(const char* filePath) {
        ma_result result = ma_sound_init_from_file(&g_engine, filePath, MA_SOUND_FLAG_DECODE, nullptr, nullptr, &sound);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize audio sound.");
        }
        assert(result == MA_SUCCESS);
    }

    void Play() {
        ma_sound_start(&sound);
    }

    void Stop() {
        ma_sound_stop(&sound);
    }

    void Reset() {
        ma_sound_seek_to_pcm_frame(&sound, 0);
    }

    ma_sound sound{};
};
