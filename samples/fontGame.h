#ifndef GAMEENGINE_FONTGAME_H
#define GAMEENGINE_FONTGAME_H

#include "../engine/game.h"
#include "string"

class FontGame : public Game {
public:
    using Game::Game;

    ~FontGame() override {
        delete burbank;
        delete font;
    };

    float size = 0.2f;

    void Start() override {
        platform->LoadShaders();

        burbank = platform->CreateSprite("assets/sprites/burbank512.png");
        burbank->transform.width = size;
        burbank->transform.height = size;
        burbank->useAtlas = true;
        burbank->atlasNumRows = 8;
        burbank->atlasCellSize = 0.125f;
        burbank->atlasRow = 0;
        burbank->atlasColumn = 0;

        font = platform->CreateSprite("assets/sprites/font512.png");
        font->transform.width = size/2;
        font->transform.height = size/2;
        font->useAtlas = true;
        burbank->atlasNumRows = 8;
        font->atlasCellSize = 0.125f;
        font->atlasRow = 0;
        font->atlasColumn = 0;
    }

    void Update() override {
        ShowText("PONG", burbank, size, { -0.18, 0.2});
        ShowText("PRESS ENTER", font, size/2, {-0.2, -0.2});
    }

    static void ShowText(const std::string& text, Sprite* atlas, float fontSize, Vector3 pos) {
        int i = 0;
        float yPos = pos.y;
        int horOffset = i;

        for (auto c : text) {
            if (c == '\n') {
                yPos -= fontSize;
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
            atlas->atlasRow = row;
            atlas->atlasColumn = col;
            atlas->transform.pos.y = yPos;
            atlas->transform.pos.x = pos.x + (horOffset * (fontSize - fontSize/2.5));
            i++;
            horOffset++;
            atlas->Update();
        }
    }

private:
    static const int CHAR_COUNT = 64;
    Sprite* burbank;
    Sprite* font;
    Sprite* spriteChars[CHAR_COUNT];
    Sprite* spriteChars2[CHAR_COUNT];
};


#endif //GAMEENGINE_FONTGAME_H
