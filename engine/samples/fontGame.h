#ifndef GAMEENGINE_FONTGAME_H
#define GAMEENGINE_FONTGAME_H

#include "../engine/game.h"
#include "../engine/text.h"
#include "string"

class FontGame : public Game {
public:
    using Game::Game;

    ~FontGame() override {
        delete burbank;
        delete font;
    };

    void Start() override {
        platform->LoadShaders();

        burbank = platform->CreateSpriteAtlas("assets/sprites/burbank2048.png", 0, 0, size, size, atlasNumRows, atlasCellSize);
        font = platform->CreateSpriteAtlas("assets/sprites/font2048.png", 0, 0, size, size, atlasNumRows, atlasCellSize);

        const char* texture = "assets/sprites/burbank2048.png";
        sprites[0] = burbank;
        sprites[1] = font;
        sprites[2] = platform->CreateSpriteAtlas(texture, 0, 0, size/3, size/3, atlasNumRows, atlasCellSize);
        sprites[3] = platform->CreateSpriteAtlas(texture, 0, 0, size/2, size/2, atlasNumRows, atlasCellSize);
        sprites[4] = platform->CreateSpriteAtlas(texture, 0, 0, size/1.5, size/1.5, atlasNumRows, atlasCellSize);
        sprites[5] = platform->CreateSpriteAtlas(texture, 0, 0, size/1.25, size/1.25, atlasNumRows, atlasCellSize);
        sprites[6] = platform->CreateSpriteAtlas(texture, 0, 0, size, size, atlasNumRows, atlasCellSize);
        sprites[7] = platform->CreateSpriteAtlas(texture, 0, 0, size/15, size/15, atlasNumRows, atlasCellSize);
        sprites[8] = platform->CreateSpriteAtlas(texture, 0, 0, size/10, size/10, atlasNumRows, atlasCellSize);
        sprites[9] = platform->CreateSpriteAtlas(texture, 0, 0, size/5, size/5, atlasNumRows, atlasCellSize);

        EnableCallback(MOUSE_RELEASED);
    }

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        if (key == MouseButton::Left) {
            tracking += 0.1;
        } else if (key == MouseButton::Right) {
            tracking -= 0.1;

        }
    }

    void Update() override {
        ShowText("right", sprites[2], {{ 0.9, 0.8 }, ParagraphAlignment::RIGHT, size/3, tracking});

        ShowText("right", sprites[3], {{ 0.9, 0.6 }, ParagraphAlignment::RIGHT, size/2, tracking});

        ShowText("right", sprites[4], {{ 0.9, 0.3}, ParagraphAlignment::RIGHT, size/1.5f, tracking });

        ShowText("right", sprites[5], {{ 0.9, 0}, ParagraphAlignment::RIGHT, size/1.25f, tracking });

        ShowText("sphinx of black quartz", sprites[7], {{ -0.9, 0.3}, ParagraphAlignment::LEFT, size/15.0f, tracking });

        ShowText("sphinx of black quartz", sprites[9], {{ -0.9, -0.3}, ParagraphAlignment::LEFT, size/5.0f, tracking });

        ShowText("sphinx of black quartz", sprites[8], {{ -0.9, 0}, ParagraphAlignment::LEFT, size/10.0f, tracking });

        snprintf(buffer, sizeof(buffer),  "Tracking:%g\n",tracking);

        ShowText(buffer, sprites[8], {{ 0, 0.9}, ParagraphAlignment::MIDDLE, size/10.0f, 0.5});
    }

private:
    Sprite* burbank;
    Sprite* font;
    Sprite* sprites[10]{};

    float size = 0.4f;
    int atlasNumRows = 8;
    float atlasCellSize = 0.125f;
    char buffer[100];
    float tracking = 1;
};


#endif //GAMEENGINE_FONTGAME_H
