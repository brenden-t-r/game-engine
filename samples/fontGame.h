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
        font->transform.width = size/3;
        font->transform.height = size/3;
        font->useAtlas = true;
        font->atlasNumRows = 8;
        font->atlasCellSize = 0.125f;
        font->atlasRow = 0;
        font->atlasColumn = 0;

        const char* spritef = "assets/sprites/burbank2048.png";

        sprites[0] = burbank;
        sprites[1] = font;
        sprites[2] = platform->CreateSprite(spritef);
        sprites[2]->transform.width = size/3;
        sprites[2]->transform.height = size/3;
        sprites[2]->useAtlas = true;
        sprites[2]->atlasNumRows = 8;
        sprites[2]->atlasCellSize = 0.125f;
        sprites[2]->atlasRow = 0;
        sprites[2]->atlasColumn = 0;

        sprites[3] = platform->CreateSprite(spritef);
        sprites[3]->transform.width = size/2;
        sprites[3]->transform.height = size/2;
        sprites[3]->useAtlas = true;
        sprites[3]->atlasNumRows = 8;
        sprites[3]->atlasCellSize = 0.125f;
        sprites[3]->atlasRow = 0;
        sprites[3]->atlasColumn = 0;

        sprites[4] = platform->CreateSprite(spritef);
        sprites[4]->transform.width = size/1.5;
        sprites[4]->transform.height = size/1.5;
        sprites[4]->useAtlas = true;
        sprites[4]->atlasNumRows = 8;
        sprites[4]->atlasCellSize = 0.125f;
        sprites[4]->atlasRow = 0;
        sprites[4]->atlasColumn = 0;

        sprites[5] = platform->CreateSprite(spritef);
        sprites[5]->transform.width = size/1.25;
        sprites[5]->transform.height = size/1.25;
        sprites[5]->useAtlas = true;
        sprites[5]->atlasNumRows = 8;
        sprites[5]->atlasCellSize = 0.125f;
        sprites[5]->atlasRow = 0;
        sprites[5]->atlasColumn = 0;

        sprites[6] = platform->CreateSprite(spritef);
        sprites[6]->transform.width = size;
        sprites[6]->transform.height = size;
        sprites[6]->useAtlas = true;
        sprites[6]->atlasNumRows = 8;
        sprites[6]->atlasCellSize = 0.125f;
        sprites[6]->atlasRow = 0;
        sprites[6]->atlasColumn = 0;

        sprites[7] = platform->CreateSprite(spritef);
        sprites[7]->transform.width = size/15.0;
        sprites[7]->transform.height = size/15.0;
        sprites[7]->useAtlas = true;
        sprites[7]->atlasNumRows = 8;
        sprites[7]->atlasCellSize = 0.125f;
        sprites[7]->atlasRow = 0;
        sprites[7]->atlasColumn = 0;

        sprites[8] = platform->CreateSprite(spritef);
        sprites[8]->transform.width = size/10;
        sprites[8]->transform.height = size/10;
        sprites[8]->useAtlas = true;
        sprites[8]->atlasNumRows = 8;
        sprites[8]->atlasCellSize = 0.125f;
        sprites[8]->atlasRow = 0;
        sprites[8]->atlasColumn = 0;

        sprites[9] = platform->CreateSprite(spritef);
        sprites[9]->transform.width = size/5;
        sprites[9]->transform.height = size/5;
        sprites[9]->useAtlas = true;
        sprites[9]->atlasNumRows = 8;
        sprites[9]->atlasCellSize = 0.125f;
        sprites[9]->atlasRow = 0;
        sprites[9]->atlasColumn = 0;
    }

    char buffer[100];
    float tracking = 1;

    void Update() override {
        burbank->transform.width = size;
        burbank->transform.height = size;

        ShowText("right", sprites[2], {ParagraphAlignment::RIGHT, size/3, { 0.9, 0.8 }, tracking});

        ShowText("right", sprites[3], {ParagraphAlignment::RIGHT, size/2, { 0.9, 0.6 }, tracking});

        ShowText("right", sprites[4], {ParagraphAlignment::RIGHT, size/1.5f, { 0.9, 0.3}, tracking });

        ShowText("right", sprites[5], {ParagraphAlignment::RIGHT, size/1.25f, { 0.9, 0}, tracking });

        ShowText("sphinx of black quartz", sprites[7], {ParagraphAlignment::LEFT, size/15.0f, { -0.9, 0.3}, tracking });

        ShowText("sphinx of black quartz", sprites[9], {ParagraphAlignment::LEFT, size/5.0f, { -0.9, -0.3}, tracking });

        ShowText("sphinx of black quartz", sprites[8], {ParagraphAlignment::LEFT, size/10.0f, { -0.9, 0}, tracking });

        snprintf(buffer, sizeof(buffer),  "Tracking:%g\n",tracking);

        ShowText(buffer, sprites[8], {ParagraphAlignment::MIDDLE, size/10.0f, { 0, 0.9}, 0.5});
        if (platform->IsMouseReleased(MouseButton::Left)) {
            tracking += 0.1;
        }
        if (platform->IsMouseReleased(MouseButton::Right)) {
            tracking -= 0.1;
        }
    }

    enum ParagraphAlignment{
        LEFT, MIDDLE, RIGHT
    };
    struct ParagraphSettings{
        ParagraphAlignment alignment;
        float fontSize;
        Vector3 pos;
        float tracking;
    };

    static void ShowText(const std::string& text, Sprite* atlas, ParagraphSettings settings) {
        int i = 0;
        float xPos = settings.pos.x;
        float yPos = settings.pos.y;
        float trackingWidth = settings.fontSize - settings.tracking*settings.fontSize;
        float letterWidth = settings.fontSize - trackingWidth;
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
            atlas->transform.pos.x = xPos + (horOffset * (settings.fontSize - trackingWidth));
            i++;
            horOffset++;
            atlas->Update();
        }
    }

private:
    Sprite* burbank;
    Sprite* font;
    Sprite* sprites[10]{};
};


#endif //GAMEENGINE_FONTGAME_H
