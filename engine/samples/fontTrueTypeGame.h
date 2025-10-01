#ifndef GAMEENGINE_FONTTRUETYPEGAME_H
#define GAMEENGINE_FONTTRUETYPEGAME_H

#include "../engine/game.h"
#include "../engine/text.h"
#include "../engine/text_msdf.h"

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

std::string LoadFileData(const char* path) {
    std::ifstream file(path);
    if (!file) {
        assert(false);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();
    return buffer.str();
}

class FontTrueTypeGame : public Game {
public:
    using Game::Game;

    ~FontTrueTypeGame() override {};

    Sprite* LoadFont(const char* pngPath, const char* jsonPath, TEXT_MSDF::FontAtlas& _atlas) {
        auto sprite = platform->CreateSprite(pngPath);
        std::string data = LoadFileData(jsonPath);
        _atlas = TEXT_MSDF::fromJsonFontAtlas(data.c_str());
        sprite->useAtlas = true;
        sprite->useGlyph = true;
        sprite->atlasWidth = _atlas.atlas.width;
        sprite->atlasHeight = _atlas.atlas.height;
        sprite->transform.width = atlas.atlas.width * 2.0 / (WINDOW_WIDTH*1.0);
        sprite->transform.height = atlas.atlas.height * 2.0 / (WINDOW_HEIGHT*1.0);
        sprite->transform.scale = {1, 1, 1};
        sprite->transform.pos = {0,0,0};
        return sprite;
    }
    Sprite* LoadFont(const char* name, TEXT_MSDF::FontAtlas& _atlas) {
        std::string png = "assets/sprites/fonts/" + std::string(name) + ".png";
        std::string json = "assets/sprites/fonts/" + std::string(name) + ".json";
        return LoadFont(png.c_str(), json.c_str(), _atlas);
    }

    TEXT_MSDF::FontAtlas atlas;
    TEXT_MSDF::FontAtlas atlas50;
    TEXT_MSDF::FontAtlas atlas25;
    TEXT_MSDF::FontAtlas atlas10;
    TEXT_MSDF::FontAtlas atlas15;
    TEXT_MSDF::FontAtlas atlas8;
    Sprite* font15;
    Sprite* font8;


    void Start() override {
        platform->LoadShaders();

        const char* png = "assets/sprites/fonts/arial.png";
        font = LoadFont("arial", atlas);
        font50 = LoadFont("arial_512", atlas50);
        font25 = LoadFont("arial_25", atlas25);
        font10 = LoadFont("arial_10", atlas10);
        font8 = LoadFont("arial_8", atlas8);
        font15 = LoadFont("arial_15", atlas15);
        font15->transform.width = 2;
        font15->transform.height = 2;

        sprites[0] = platform->CreateSprite(png);
        sprites[0]->transform.width = (float)atlas.atlas.width / (float)WINDOW_WIDTH;
        sprites[0]->transform.height = (float)atlas.atlas.height / (float)WINDOW_HEIGHT;
        sprites[0]->transform.scale = {1,1,1};
        sprites[0]->transform.pos = {0,0,0};

        auto gameObject = platform->CreateGameObject();
        gameObject->transform.rot.z = 45;
//        font->transform.parent = &gameObject->transform;
//        font50->transform.parent = &gameObject->transform;
//        font25->transform.parent = &gameObject->transform;
//        font10->transform.parent = &gameObject->transform;

        font->transform.pos = {-0.5, 0.7, 1};
        font->transform.scale = {1, 1, 1}; //100
    }

    void Update() override {
//        sprites[0]->Update();
        //font->Update();

        // Scale == 1, atlas is 1024, font is 100.
//        font->transform.pos = {-0.5, 0.7, 1};
//        font->transform.scale = {1, 1, 1}; //100
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};", -0.9, 0.7, atlas);
        font50->transform.pos = {-0.5, -0.1, 1};
        font50->transform.scale = {1, 1, 1}; // 40
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};", -0.1, -0.1, atlas50);
        font25->transform.pos = {-0.5, -0.7, 1};
        font25->transform.scale = {1.0, 1.0, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};", -0.1, -0.7, atlas25);
        font10->transform.pos = {-0.5, -0.8, 1};
        font10->transform.scale = {1.0, 1.0, 1}; // 5
        RenderTextMSDF(font10, "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.", 0.3, -0.8,atlas10);
        font15->transform.pos = {-0.5, -0.8, 1};
        font15->transform.scale = {1.0, 1.0, 1}; // 5
        RenderTextMSDF(font15, "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.", -0.8, -0.8,atlas15);
        font8->transform.pos = {-0.5, -0.85, 1};
        font8->transform.scale = {1.0, 1.0, 1}; // 5
        RenderTextMSDF(font8, "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.", 0.1, -0.85,atlas8);
        //font->Update();
        if (platform->IsKeyPressed(KeyCode::Up)) {
            font->transform.scale.x += 0.01f;
            font->transform.scale.y += 0.01f;
        }
        if (platform->IsKeyPressed(KeyCode::Down)) {
            font->transform.scale.x -= 0.01f;
            font->transform.scale.y -= 0.01f;
        }
        if (platform->IsKeyPressed(KeyCode::Right)) {
            font->transform.rot.z += 1;
        }
        if (platform->IsKeyPressed(KeyCode::Left)) {
            font->transform.rot.z -= 1;
        }
    }

    void RenderTextMSDF(Sprite* _font, const std::string &text, float startX, float startY, TEXT_MSDF::FontAtlas _atlas) {
        float x = startX;
        float y = startY;
        //float y = startY + baseline * scale;  // align baseline

        for (char ch : text) {
            TEXT_MSDF::Glyph g;
            if (ch == '\n') {
                x = startX;
                y -= round(_atlas.atlas.size*_atlas.metrics.lineHeight*_font->transform.scale.y) / WINDOW_HEIGHT*2.0f;
                continue;
            }
            bool found = false;
            for (TEXT_MSDF::Glyph gly : _atlas.glyphs) {
                if (gly.unicode == ch) {
                    g = gly;
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
//                return;
            }

            _font->glyphX = g.atlasBounds.left;
            _font->glyphY = _atlas.atlas.height - g.atlasBounds.top;
            _font->glyphW = (g.atlasBounds.right - g.atlasBounds.left);
            _font->glyphH = (_atlas.atlas.height - g.atlasBounds.bottom) - (_atlas.atlas.height - g.atlasBounds.top);
            _font->glyphxoff = (g.planeBounds.left * _atlas.atlas.size) / WINDOW_WIDTH * 2.0f;
            _font->glyphyoff = (g.planeBounds.top * _atlas.atlas.size) / WINDOW_HEIGHT *2.0f;
            _font->glyphMsdf = g;
            _font->transform.pos.x = x;
            _font->transform.pos.y = y;
            _font->Update();
            float pixelAdvance = g.advance * _atlas.atlas.size * 2.0f;
            x += (pixelAdvance/WINDOW_WIDTH*_font->transform.scale.x*1.0);
        }
    }

private:
    Sprite* burbank;
    Sprite* font;
    Sprite* font50;
    Sprite* font25;
    Sprite* font10;
    Sprite* sprites[10]{};

    float size = 0.4f;
    int atlasNumRows = 8;
    float atlasCellSize = 0.125f;
    char buffer[100];
    float tracking = 1;
};


#endif //GAMEENGINE_FONTTRUETYPEGAME_H
