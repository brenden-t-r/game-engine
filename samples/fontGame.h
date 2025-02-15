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

    float size = 0.4f;

    void Start() override {
        platform->LoadShaders();

        burbank = platform->CreateSprite("assets/sprites/burbank2048.png");
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
        ShowText("PONG", burbank, {ParagraphAlignment::MIDDLE, size, { 0, 0.2 }});
        ShowText("PRESS ENTER", font,  {ParagraphAlignment::MIDDLE, size/2, { 0, -0.2 }});

        ShowText("LEFT", font, {ParagraphAlignment::LEFT, size/2, { -0.9, 0.8 }});
        ShowText("RIGHT", font, {ParagraphAlignment::RIGHT, size/2, { 0.9, 0.8 }});
    }

    enum ParagraphAlignment{
        LEFT, MIDDLE, RIGHT
    };
    struct ParagraphSettings{
        ParagraphAlignment alignment;
        float fontSize;
        Vector3 pos;
    };

    static void ShowText(const std::string& text, Sprite* atlas, ParagraphSettings settings) {
        int i = 0;
        float xPos = settings.pos.x;
        float yPos = settings.pos.y;
        float letterWidth = settings.fontSize - settings.fontSize/2.5;
        float lineWidth = letterWidth * text.size();

        if (settings.alignment == ParagraphAlignment::MIDDLE) {
            xPos = xPos - lineWidth/2 + letterWidth/2;
        }
        if (settings.alignment == ParagraphAlignment::RIGHT) {
            xPos = xPos - lineWidth + letterWidth;
        }

        int horOffset = i;

        for (auto c : text) {
            if (c == '\n') {
                yPos -= settings.fontSize;
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
            atlas->transform.pos.x = xPos + (horOffset * (settings.fontSize - settings.fontSize/2.5));
            i++;
            horOffset++;
            atlas->Update();
        }
    }

private:
    Sprite* burbank;
    Sprite* font;
};


#endif //GAMEENGINE_FONTGAME_H
