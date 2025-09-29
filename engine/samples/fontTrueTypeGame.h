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

    void Start() override {
        platform->LoadShaders();


//        const char* txt = "assets/sprites/fonts/burbank_atlas.txt";
//        const char* png = "assets/sprites/fonts/burbank_atlas.png";
//        const char* txt = "assets/sprites/fonts/arial_atlas.txt";
        const char* png = "assets/sprites/fonts/arial.png";
//        const char* png = "assets/sprites/cardaction.png";

        font = platform->CreateSprite(png);

        // MSDF atlas
        std::string data = LoadFileData("assets/sprites/fonts/arial.json");
        atlas = TEXT_MSDF::fromJsonFontAtlas(data.c_str());

// True type bitmap
//        LoadFontMeta(txt);
        font->useAtlas = true;
        font->useGlyph = true;
//        font->glyphW = 0;
//        font->glyphH = 0;-+9
//        font->glyphX = 0;
//        font->glyphY = 0;
        font->atlasWidth = atlas.atlas.width;
        font->atlasHeight = atlas.atlas.height;
        font->transform.width = (float)atlas.atlas.width / (float)WINDOW_WIDTH;
        font->transform.height = (float)atlas.atlas.height / (float)WINDOW_HEIGHT;
        font->transform.scale = {1, 1, 1};
        font->transform.pos = {0,0,0};

        sprites[0] = platform->CreateSprite(png);
        sprites[0]->transform.width = (float)atlas.atlas.width / (float)WINDOW_WIDTH;
        sprites[0]->transform.height = (float)atlas.atlas.height / (float)WINDOW_HEIGHT;
        sprites[0]->transform.scale = {1,1,1};
        sprites[0]->transform.pos = {0,0,0};

        auto gameObject = platform->CreateGameObject();
        gameObject->transform.rot.z = 45;
        //font->transform.parent = &gameObject->transform;
    }

    TEXT_MSDF::FontAtlas atlas;
    void Update() override {
        sprites[0]->Update();
//        font->Update();

        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, 0.2, 1};
        font->transform.scale = {2, 2, 1};
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, -0.2, 1};
        font->transform.scale = {3.5, 3.5, 1};
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.5, -0.5, 1};
        font->transform.scale = {0.25, 0.25, 1};
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.9, -0.8, 1};
        font->transform.scale = {0.18, 0.18, 1};
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
        font->transform.pos = {-0.9, -0.9, 1};
        font->transform.scale = {0.5, 0.5, 1};
        RenderTextMSDF(font, "@sphinx of black quartz, judge my vow.", -0.9, atlas);
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
    Sprite* sprites[10]{};

    float size = 0.4f;
    int atlasNumRows = 8;
    float atlasCellSize = 0.125f;
    char buffer[100];
    float tracking = 1;
};


#endif //GAMEENGINE_FONTTRUETYPEGAME_H
