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

    ~FontTrueTypeGame() override = default;

    Shader* LoadFontShader() {
        ShaderDef shaderDef{};
#if BACKEND_DIRECTX
        shaderDef.path = "assets/shaders/font.hlsl";
        shaderDef.inputLayoutType = InputLayoutType::POSITION_TEXCOORD;
#elif BACKEND_OPENGL
        shaderDef.vertexPath = "assets/shaders/font.glsl.vert";
        shaderDef.fragmentPath = "assets/shaders/font.glsl.frag";
#elif BACKEND_METAL
#endif
        return platform->LoadShader(shaderDef);
    }
    Sprite* LoadFont(const char* name, TEXT_MSDF::FontAtlas& _atlas, Shader* shader, TextureSettings textureSettings) {
        std::string png = "assets/sprites/fonts/" + std::string(name) + ".png";
        std::string json = "assets/sprites/fonts/" + std::string(name) + ".json";
        return LoadFont(png.c_str(), json.c_str(), _atlas, shader, textureSettings);
    }
    Sprite* LoadFont(const char* pngPath, const char* jsonPath,
                     TEXT_MSDF::FontAtlas& _atlas, Shader* shader, TextureSettings textureSettings) {
        auto texture = platform->CreateTexture(pngPath, textureSettings);
        auto sprite = platform->CreateSprite(texture);
        auto mat = new MaterialFont(shader, texture);
        sprite->SetMaterial(mat);
        std::string data = LoadFileData(jsonPath);
        _atlas = TEXT_MSDF::fromJsonFontAtlas(data.c_str());
        sprite->useAtlas = true;
        sprite->useGlyph = true;
        sprite->atlasWidth = (float)_atlas.atlas.width;
        sprite->atlasHeight = (float)_atlas.atlas.height;
        sprite->transform.width = (float)atlas.atlas.width * 2.0f / ((float)WINDOW_WIDTH*1.0f);
        sprite->transform.height = (float)atlas.atlas.height * 2.0f / ((float)WINDOW_HEIGHT*1.0f);
        sprite->transform.scale = {1, 1, 1};
        sprite->transform.pos = {0,0,0};
        return sprite;
    }

    void Start() override {
        platform->LoadShaders();
        auto shader = LoadFontShader();
        auto linearNoMips = TextureSettings{TextureFilter::LINEAR, false};
        auto pointNoMips = TextureSettings{TextureFilter::POINT, false};
        font = LoadFont("arial", atlas, shader, linearNoMips);
        font->transform.pos = {-0.5, 0.7, 1};
        font->transform.scale = {1, 1, 1};
        auto colorMaterial = (MaterialColor*)font->material;
        colorMaterial->color[0] = 0.0;
        colorMaterial->color[3] = 0.5;
        font50 = LoadFont("arial_50", atlas50, shader, pointNoMips);
        font25 = LoadFont("arial_25", atlas25, shader, pointNoMips);
        font10 = LoadFont("arial_10", atlas10, shader, pointNoMips);
        font8 = LoadFont("arial_8", atlas8, shader, pointNoMips);
        font15 = LoadFont("arial_15", atlas15, shader, pointNoMips);
        font16 = LoadFont("arial_16", atlas16, shader, pointNoMips);
        fontMSDF = LoadFont("arial_msdf", atlasMSDF, shader, linearNoMips);
        auto mat = (MaterialFont*)fontMSDF->material;
        mat->color[0] = 0.0;
        mat->color[1] = 1.0;
        mat->color[2] = 0.0;
        mat->outlineColor[0] = 0.6;
        mat->outlineColor[1] = 0.6;
        mat->outlineColor[2] = 1.0;
        mat->outlineColor[3] = 0.6;
        mat->outlineWidth = 8;
        mat->pxRange = 16;
        mat->isMSDF = true;
    }

    void Update() override {
        auto mat = (MaterialFont*)fontMSDF->material;
        if (mat->outlineColor[ind] > 0.9) {
            dir = -1;
        } else if (mat->outlineColor[ind] < 0.1) {
            dir = 1;
            ind = ind == 2 ? 1 : 2;
        }
        mat->outlineColor[ind] += 0.05f * dir;

        RenderText(font,
                   "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};",
                   -0.9, 0.7, atlas);
        font50->transform.pos = {-0.5, -0.1, 1};
        font50->transform.scale = {1, 1, 1}; // 40
        RenderText(font50,
                   "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};",
                   -0.1, -0.1, atlas50);
        font25->transform.pos = {-0.5, -0.7, 1};
        font25->transform.scale = {1.0, 1.0, 1}; // 5
        RenderText(font25,
                   "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};",
                   -0.1, -0.7, atlas25);
        font10->transform.pos = {-0.5, -0.8, 1};
        font10->transform.scale = {1.0, 1.0, 1}; // 5
        RenderText(font10,
                   "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.",
                   0.3, -0.8, atlas10);
        font15->transform.pos = {-0.5, -0.8, 1};
        font15->transform.scale = {1.0, 1.0, 1}; // 5
        RenderText(font15,
                   "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.",
                   -0.8, -0.8, atlas15);
        font8->transform.pos = {-0.5, -0.85, 1};
        font8->transform.scale = {1.0, 1.0, 1}; // 5
        RenderText(font8,
                   "FONT SIZE 8 (EIGHT) @sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.",
                   -0.3, -0.8, atlas8);
        font16->transform.pos = {-0.5, -0.85, 1};
        font16->transform.scale = {1.0, 1.0, 1}; // 5
        RenderText(font16,
                   "FONT SIZE 16 @sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, \nJUDGE MY VOW 0123456789!@#$%^&*()[]{};\n@sphinx of black quartz, judge my vow.",
                   -0.5, -0.3, atlas16);

        RenderText(fontMSDF,
                   "@sphinx of black quartz, judge my vow.\nSPHINX OF BLACK QUARTZ, JUDGE MY VOW 0123456789!@#$%^&*()[]{};",
                   -0.9, 0.1, atlasMSDF);

        if (platform->IsKeyPressed(KeyCode::Up)) {
            font->transform.scale.x += 0.01f;
            font->transform.scale.y += 0.01f;
            fontMSDF->transform.scale.x += 0.01f;
            fontMSDF->transform.scale.y += 0.01f;
        }
        if (platform->IsKeyPressed(KeyCode::Down)) {
            font->transform.scale.x -= 0.01f;
            font->transform.scale.y -= 0.01f;
            fontMSDF->transform.scale.x -= 0.01f;
            fontMSDF->transform.scale.y -= 0.01f;
        }
        if (platform->IsKeyPressed(KeyCode::Right)) {
            font->transform.rot.z += 1;
            fontMSDF->transform.rot.z += 1;
        }
        if (platform->IsKeyPressed(KeyCode::Left)) {
            font->transform.rot.z -= 1;
            fontMSDF->transform.rot.z -= 1;
        }
    }

    static void RenderText(Sprite* _font, const std::string &text, float startX, float startY, const TEXT_MSDF::FontAtlas& _atlas) {
        float x = startX;
        float y = startY;
        //float y = startY + baseline * scale;  // align baseline

        for (char ch : text) {
            TEXT_MSDF::Glyph g{};
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
            }

            _font->glyphX = (float)g.atlasBounds.left;
            _font->glyphY = (float)_atlas.atlas.height - (float)g.atlasBounds.top;
            _font->glyphW = (float)(g.atlasBounds.right - g.atlasBounds.left);
            _font->glyphH = (float)(_atlas.atlas.height - g.atlasBounds.bottom) - (float)(_atlas.atlas.height - g.atlasBounds.top);
            _font->glyphxoff = (float)(g.planeBounds.left * _atlas.atlas.size) / (float)WINDOW_WIDTH * 2.0f;
            _font->glyphyoff = (float)(g.planeBounds.top * _atlas.atlas.size) / (float)WINDOW_HEIGHT *2.0f;
            _font->transform.pos.x = x;
            _font->transform.pos.y = y;
            _font->Update();
            auto pixelAdvance = (float)(g.advance * _atlas.atlas.size * 2.0f);
            pixelAdvance += 2;
            x += (float)(pixelAdvance/(float)WINDOW_WIDTH*_font->transform.scale.x*1.0);
        }
    }

private:
    Sprite* font;
    Sprite* font50;
    Sprite* font25;
    Sprite* font15;
    Sprite* font16;
    Sprite* font10;
    Sprite* font8;
    Sprite* fontMSDF;
    TEXT_MSDF::FontAtlas atlas;
    TEXT_MSDF::FontAtlas atlas50;
    TEXT_MSDF::FontAtlas atlas25;
    TEXT_MSDF::FontAtlas atlas10;
    TEXT_MSDF::FontAtlas atlas15;
    TEXT_MSDF::FontAtlas atlas16;
    TEXT_MSDF::FontAtlas atlas8;
    TEXT_MSDF::FontAtlas atlasMSDF;
    char buffer[100];
    float size = 0.4f;
    float dir = 1;
    int ind = 2;
};


#endif //GAMEENGINE_FONTTRUETYPEGAME_H
