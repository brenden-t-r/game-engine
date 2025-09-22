#ifndef GAMEENGINE_FONTTRUETYPEGAME_H
#define GAMEENGINE_FONTTRUETYPEGAME_H

#include "../engine/game.h"
#include "../engine/text.h"

#include "string"
#include "unordered_map"
#include "fstream"


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


class FontTrueTypeGame : public Game {
public:
    using Game::Game;

    ~FontTrueTypeGame() override {

    };

    void Start() override {
        platform->LoadShaders();


//        const char* txt = "assets/sprites/fonts/burbank_atlas.txt";
//        const char* png = "assets/sprites/fonts/burbank_atlas.png";
        const char* txt = "assets/sprites/fonts/arial_atlas.txt";
        const char* png = "assets/sprites/fonts/arial_atlas.png";
//        png = "assets/sprites/fonts/arial_atlas64_15.png";
//        txt = "assets/sprites/fonts/arial_atlas64_15.txt";

        LoadFontMeta(txt);

        font = platform->CreateSprite(png);
        font->useAtlas = true;
        font->useGlyph = true;
        font->glyphW = 0;
        font->glyphH = 0;
        font->glyphX = 0;
        font->glyphY = 0;
        font->atlasWidth = atlasWidth;
        font->atlasHeight = atlasHeight;
        font->transform.scale = {1, 1, 1};

//        sprites[0] = platform->CreateSprite("assets/sprites/fonts/arial_atlas64_15.png");
//        sprites[0]->transform.width = 64.0/WINDOW_WIDTH*2.0;
//        sprites[0]->transform.height = 64.0/WINDOW_HEIGHT*2.0;
//        sprites[0]->transform.pos = vec3{0,0,0};
//
//        sprites[1] = platform->CreateSprite("assets/sprites/fonts/arial_atlas64_15.png");
//        sprites[1]->transform.width = 64.0/WINDOW_WIDTH*1.5;
//        sprites[1]->transform.height = 64.0/WINDOW_HEIGHT*1.5;
//        sprites[1]->transform.pos = vec3{-0.4,0,0};
//
//        sprites[2] = platform->CreateSprite("assets/sprites/fonts/arial_atlas64_15.png");
//        sprites[2]->transform.width = 64.0/WINDOW_WIDTH*1.75;
//        sprites[2]->transform.height = 64.0/WINDOW_HEIGHT*1.75;
//        sprites[2]->transform.pos = vec3{-0.2,0,0};

        sprites[3] = platform->CreateSprite(png);
        sprites[3]->transform.width = 64.0/WINDOW_WIDTH*2.0;
        sprites[3]->transform.height = 64.0/WINDOW_HEIGHT*2.0;
        sprites[3]->transform.pos = vec3{-0.2,0.4,0};


        auto gameObject = platform->CreateGameObject();
        gameObject->transform.rot.z = 45;
        //font->transform.parent = &gameObject->transform;

    }

    void Update() override {
//                sprites[0]->Update();
//                sprites[1]->Update();
//                sprites[2]->Update();
                sprites[3]->Update();
//                return;
        font->transform.pos = {-0.5, 0.5, 1};
        font->transform.scale = {1.9, 1.9, 1};
        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);



//        font->transform.pos = {-0.5, 0.5, 1};
//        font->transform.scale = {1, 1, 1};
//        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);
        font->transform.pos = {-0.5, 0.2, 1};
        font->transform.scale = {2, 2, 1};
        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);
        font->transform.pos = {-0.5, -0.2, 1};
        font->transform.scale = {3.5, 3.5, 1};
        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);
        font->transform.pos = {-0.5, -0.5, 1};
        font->transform.scale = {0.25, 0.25, 1};
        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);
        font->transform.pos = {-0.9, -0.8, 1};
        font->transform.scale = {0.18, 0.18, 1};
        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);
        font->transform.pos = {-0.9, -0.9, 1};
        font->transform.scale = {0.5, 0.5, 1};
        RenderText(font, "@sphinx of black quartz, judge my vow. SPHINX OF BLACK QUARTZ, JUDGE MY VOW. $100,234,567,890 !@#$%^&*()-_=+/[]{};'`'<>", -0.9);
        //font->Update();
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
            x += (g.advance/WINDOW_WIDTH*font->transform.scale.x*1.3);
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
