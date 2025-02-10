#ifndef GAMEENGINE_FONTGAME_H
#define GAMEENGINE_FONTGAME_H

#include "../engine/game.h"
#include "string"

class FontGame : public Game {
public:
    using Game::Game;

    ~FontGame() override {
        for (auto & spriteChar : spriteChars) {
            delete spriteChar;  // Free allocated memory
        }
        for (auto & spriteChar : spriteChars2) {
            delete spriteChar;  // Free allocated memory
        }
    };

    float size = 0.2f;
    float textPosX = -0.18;
    float textPosY = 0.2;

    void Start() override {
        platform->LoadShaders();

        int i = 0;

        for (auto c : AllCharacters) {
            spriteChars[i] = platform->CreateSprite("assets/sprites/burbank512.png");
            spriteChars[i]->transform.width = size;
            spriteChars[i]->transform.height = size;
            spriteChars[i]->atlasCellSize = 0.125f;
            spriteChars[i]->atlasRow = (i / 8);
            spriteChars[i]->atlasColumn = (i % 8);
            i++;
        }
        i = 0;
        for (auto c : AllCharacters) {
            spriteChars2[i] = platform->CreateSprite("assets/sprites/font512.png");
            spriteChars2[i]->transform.width = size/2;
            spriteChars2[i]->transform.height = size/2;
            spriteChars2[i]->atlasCellSize = 0.125f;
            spriteChars2[i]->atlasRow = (i / 8);
            spriteChars2[i]->atlasColumn = (i % 8);
            i++;
        }
    }

    void Update() override {
        ShowText("PONG", spriteChars);
        ShowText("\n\nPRESS\nENTER", spriteChars2);
    }

    void ShowText(std::string text, Sprite* sprites[]) {
        int i = 0;
        float yPos = textPosY;
        int horOffset = i;

        for (auto c : text) {
            if (c == '\n') {
                yPos -= size;
                horOffset = 0;
                i++;
                continue;
            }
            int row = CharToAtlasRow(c);
            int col = CharToAtlasColumn(c);
            if (row == -1 || col == -1) {
                i++;
                horOffset++;
                continue;
            }
            auto spr = sprites[row * 8 + (col)];
            spr->transform.pos.y = yPos;
            spr->transform.pos.x = textPosX + (horOffset * (size - size/2.5));
            i++;
            horOffset++;
            spr->Update();
        }
    }

private:
    static const int CHAR_COUNT = 64;
    Sprite* spriteChars[CHAR_COUNT];
    Sprite* spriteChars2[CHAR_COUNT];
};


#endif //GAMEENGINE_FONTGAME_H
