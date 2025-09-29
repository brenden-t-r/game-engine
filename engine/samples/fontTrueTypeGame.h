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

std::unordered_map<char, Glyph> glyphs;
int atlasWidth, atlasHeight;
int baseline;

void LoadFontMeta(const std::string &path) {
    std::ifstream in(path);
    if (!in) return;
    in >> std::ws;
    std::string tag;
    in >> tag;
    if (tag == "baseline") {
        in >> baseline;
    }
    in >> tag;
    if (tag == "atlas") {
        in >> atlasWidth >> atlasHeight;
    }
    int c, x, y, w, h, xoff, yoff;
    float adv;
    while (in >> c >> x >> y >> w >> h >> xoff >> yoff >> adv) {
        Glyph g = {x, y, w, h, xoff, yoff, adv};
        glyphs[(char)c] = g;
    }
}

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

    ~FontTrueTypeGame() override {

    };

    Sprite* LoadFont(const char* pngPath, const char* jsonPath, TEXT_MSDF::FontAtlas& _atlas) {
        auto sprite = platform->CreateSprite(pngPath);
        std::string data = LoadFileData(jsonPath);
        _atlas = TEXT_MSDF::fromJsonFontAtlas(data.c_str());
        sprite->useAtlas = true;
        sprite->useGlyph = true;
        sprite->atlasWidth = _atlas.atlas.width;
        sprite->atlasHeight = _atlas.atlas.height;
        sprite->transform.width = (float)_atlas.atlas.width / (float)WINDOW_WIDTH;
        sprite->transform.height = (float)_atlas.atlas.height / (float)WINDOW_HEIGHT;
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

    void Start() override {
        platform->LoadShaders();

        const char* png = "assets/sprites/fonts/arial.png";
        font = LoadFont("arial", atlas);
        font50 = LoadFont("arial_512", atlas50);
        font25 = LoadFont("arial_25", atlas25);
        font10 = LoadFont("arial_10", atlas10);

        sprites[0] = platform->CreateSprite(png);
        sprites[0]->transform.width = (float)atlas.atlas.width / (float)WINDOW_WIDTH;
        sprites[0]->transform.height = (float)atlas.atlas.height / (float)WINDOW_HEIGHT;
        sprites[0]->transform.scale = {2,2,1};
        sprites[0]->transform.pos = {0,0,0};

        auto gameObject = platform->CreateGameObject();
        gameObject->transform.rot.z = 45;
//        font->transform.parent = &gameObject->transform;
//        font50->transform.parent = &gameObject->transform;
//        font25->transform.parent = &gameObject->transform;
//        font10->transform.parent = &gameObject->transform;
    }



    void Update() override {
//        sprites[0]->Update();
        //font->Update();

        // Scale == 1, atlas is 1024, font is 100.
        font->transform.pos = {-0.5, 0.7, 1};
        font->transform.scale = {2, 2, 1}; //100
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, 0.5, 1};
        font->transform.scale = {1.8, 1.8, 1}; //90
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, 0.35, 1};
        font->transform.scale = {1.6, 1.6, 1}; //80
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, 0.2, 1};
        font->transform.scale = {1.4, 1.4, 1}; //70
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, 0.1, 1};
        font->transform.scale = {1.2, 1.2, 1}; //60
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, 0, 1};
        font->transform.scale = {1.1, 1.1, 1}; //50
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font50->transform.pos = {-0.5, -0.1, 1};
        font50->transform.scale = {2, 2, 1}; // 40
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.2, 1};
        font50->transform.scale = {1.8, 1.8, 1}; // 35
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.3, 1};
        font50->transform.scale = {1.6, 1.6, 1}; // 30
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.4, 1};
        font50->transform.scale = {1.5, 1.5, 1}; // 25
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.5, 1};
        font50->transform.scale = {1.4, 1.4, 1}; // 20
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.6, 1};
        font50->transform.scale = {1.3, 1.3, 1}; // 15
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.7, 1};
        font50->transform.scale = {1.2, 1.2, 1}; // 10
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.8, 1};
        font50->transform.scale = {1.1, 1.1, 1}; // 5
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.9, 1};
        font50->transform.scale = {1.0, 1.0, 1}; // 5
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.95, 1};
        font50->transform.scale = {0.8, 0.8, 1}; // 5
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font50->transform.pos = {-0.5, -0.98, 1};
        font50->transform.scale = {0.6, 0.6, 1}; // 5
        RenderTextMSDF(font50, "@sphinx of black quartz, judge my vow.", -0.9, atlas50);
        font25->transform.pos = {-0.5, -0.7, 1};
        font25->transform.scale = {2.0, 2.0, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font25->transform.pos = {-0.5, -0.75, 1};
        font25->transform.scale = {1.8, 1.8, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font25->transform.pos = {-0.5, -0.8, 1};
        font25->transform.scale = {1.6, 1.6, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font25->transform.pos = {-0.5, -0.85, 1};
        font25->transform.scale = {1.4, 1.4, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font25->transform.pos = {-0.5, -0.9, 1};
        font25->transform.scale = {1.2, 1.2, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font25->transform.pos = {-0.5, -0.95, 1};
        font25->transform.scale = {1.0, 1.0, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font25->transform.pos = {-0.5, -0.97, 1};
        font25->transform.scale = {0.8, 0.8, 1}; // 5
        RenderTextMSDF(font25, "@sphinx of black quartz, judge my vow.", -0.1, atlas25);
        font10->transform.pos = {-0.5, -0.7, 1};
        font10->transform.scale = {2.0, 2.0, 1}; // 5
        RenderTextMSDF(font10, "@sphinx of black quartz, judge my vow.", 0.5, atlas10);
        font10->transform.pos = {-0.5, -0.75, 1};
        font10->transform.scale = {1.75, 1.75, 1}; // 5
        RenderTextMSDF(font10, "@sphinx of black quartz, judge my vow.", 0.5, atlas10);
        font10->transform.pos = {-0.5, -0.8, 1};
        font10->transform.scale = {1.5, 1.5, 1}; // 5
        RenderTextMSDF(font10, "@sphinx of black quartz, judge my vow.", 0.5, atlas10);
        font10->transform.pos = {-0.5, -0.85, 1};
        font10->transform.scale = {1.3, 1.3, 1}; // 5
        RenderTextMSDF(font10, "@sphinx of black quartz, judge my vow.", 0.5, atlas10);
//        font->transform.pos = {-0.5, 0.2, 1};
//        font->transform.scale = {2, 2, 1};
//        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
//        font->transform.pos = {-0.5, -0.2, 1};
//        font->transform.scale = {3.5, 3.5, 1};
//        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
//        font->transform.pos = {-0.5, -0.5, 1};
//        font->transform.scale = {0.25, 0.25, 1};
//        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
//        font->transform.pos = {-0.9, -0.8, 1};
//        font->transform.scale = {0.18, 0.18, 1};
//        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
//        font->transform.pos = {-0.9, -0.9, 1};
//        font->transform.scale = {0.5, 0.5, 1};
//        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
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

    void RenderTextMSDF(Sprite* font, const std::string &text, float startX, TEXT_MSDF::FontAtlas atlas) {
        float x = startX;
        //float y = startY + baseline * scale;  // align baseline

        for (char ch : text) {
            TEXT_MSDF::Glyph g;
            bool found = false;
            for (TEXT_MSDF::Glyph gly : atlas.glyphs) {
                if (gly.unicode == ch) {
                    g = gly;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return;
            }

            font->glyphX = g.atlasBounds.left;
            font->glyphY = atlas.atlas.height - g.atlasBounds.top;
            font->glyphW = (g.atlasBounds.right - g.atlasBounds.left);
            font->glyphH = (atlas.atlas.height - g.atlasBounds.bottom) - (atlas.atlas.height - g.atlasBounds.top);
            font->glyphxoff = (g.planeBounds.left * atlas.atlas.size) / WINDOW_WIDTH;
            font->glyphyoff = (g.planeBounds.top * atlas.atlas.size) / WINDOW_HEIGHT;
            font->glyphMsdf = g;
            font->transform.pos.x = x;
            font->Update();
            float pixelAdvance = g.advance * atlas.atlas.size;
            x += (pixelAdvance/WINDOW_WIDTH*font->transform.scale.x);
        }
    }

    void RenderText(Sprite* font, const std::string &text, float startX) {
        float x = startX;
        //float y = startY + baseline * scale;  // align baseline

        for (char ch : text) {
            auto it = glyphs.find(ch);
            if (it == glyphs.end()) continue;
            Glyph &g = it->second;

            font->glyphX = g.x;
            font->glyphY = g.y;
            font->glyphW = g.w;
            font->glyphH = g.h;
            font->glyph = g;
            font->transform.pos.x = x;
            font->Update();
            x += (g.advance/WINDOW_WIDTH*font->transform.scale.x);
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
