#ifndef GAMEENGINE_TEXT_H
#define GAMEENGINE_TEXT_H

#include <cstring>

#include "gameobject.h"

int CharToAtlasColumn(char c) {
    switch (c) {
        case 'A': return 0;
        case 'B': return 1;
        case 'C': return 2;
        case 'D': return 3;
        case 'E': return 4;
        case 'F': return 5;
        case 'G': return 6;
        case 'H': return 7;
        case 'I': return 0;
        case 'J': return 1;
        case 'K': return 2;
        case 'L': return 3;
        case 'M': return 4;
        case 'N': return 5;
        case 'O': return 6;
        case 'P': return 7;
        case 'Q': return 0;
        case 'R': return 1;
        case 'S': return 2;
        case 'T': return 3;
        case 'U': return 4;
        case 'V': return 5;
        case 'W': return 6;
        case 'X': return 7;
        case 'Y': return 0;
        case 'Z': return 1;
        case 'a': return 2;
        case 'b': return 3;
        case 'c': return 4;
        case 'd': return 5;
        case 'e': return 6;
        case 'f': return 7;
        case 'g': return 0;
        case 'h': return 1;
        case 'i': return 2;
        case 'j': return 3;
        case 'k': return 4;
        case 'l': return 5;
        case 'm': return 6;
        case 'n': return 7;
        case 'o': return 0;
        case 'p': return 1;
        case 'q': return 2;
        case 'r': return 3;
        case 's': return 4;
        case 't': return 5;
        case 'u': return 6;
        case 'v': return 7;
        case 'w': return 0;
        case 'x': return 1;
        case 'y': return 2;
        case 'z': return 3;
        case '0': return 4;
        case '1': return 5;
        case '2': return 6;
        case '3': return 7;
        case '4': return 0;
        case '5': return 1;
        case '6': return 2;
        case '7': return 3;
        case '8': return 4;
        case '9': return 5;
        case '!': return 6;
        case '?': return 7;
        default: return -1;
    }
}
int CharToAtlasRow(char c) {
    switch (c) {
        case 'A': return 0;
        case 'B': return 0;
        case 'C': return 0;
        case 'D': return 0;
        case 'E': return 0;
        case 'F': return 0;
        case 'G': return 0;
        case 'H': return 0;
        case 'I': return 1;
        case 'J': return 1;
        case 'K': return 1;
        case 'L': return 1;
        case 'M': return 1;
        case 'N': return 1;
        case 'O': return 1;
        case 'P': return 1;
        case 'Q': return 2;
        case 'R': return 2;
        case 'S': return 2;
        case 'T': return 2;
        case 'U': return 2;
        case 'V': return 2;
        case 'W': return 2;
        case 'X': return 2;
        case 'Y': return 3;
        case 'Z': return 3;
        case 'a': return 3;
        case 'b': return 3;
        case 'c': return 3;
        case 'd': return 3;
        case 'e': return 3;
        case 'f': return 3;
        case 'g': return 4;
        case 'h': return 4;
        case 'i': return 4;
        case 'j': return 4;
        case 'k': return 4;
        case 'l': return 4;
        case 'm': return 4;
        case 'n': return 4;
        case 'o': return 5;
        case 'p': return 5;
        case 'q': return 5;
        case 'r': return 5;
        case 's': return 5;
        case 't': return 5;
        case 'u': return 5;
        case 'v': return 5;
        case 'w': return 6;
        case 'x': return 6;
        case 'y': return 6;
        case 'z': return 6;
        case '0': return 6;
        case '1': return 6;
        case '2': return 6;
        case '3': return 6;
        case '4': return 7;
        case '5': return 7;
        case '6': return 7;
        case '7': return 7;
        case '8': return 7;
        case '9': return 7;
        case '!': return 7;
        case '?': return 7;
        default: return -1;
    }
}

enum ParagraphAlignment{
    LEFT, MIDDLE, RIGHT
};
struct ParagraphSettings{
    vec3 pos;
    ParagraphAlignment alignment;
    float fontSize;
    float tracking;
};

// deprecated
static void ShowText(const char* text, Sprite* atlas, ParagraphSettings settings) {
    int i = 0;
    float xPos = settings.pos.x;
    float yPos = settings.pos.y;
    float trackingWidth = settings.fontSize - settings.tracking*settings.fontSize;
    float letterWidth = settings.fontSize - trackingWidth;
    float lineWidth = letterWidth * (float)strlen(text);

    if (settings.alignment == ParagraphAlignment::MIDDLE) {
        xPos = xPos - lineWidth/2 + letterWidth/2;
    }
    if (settings.alignment == ParagraphAlignment::RIGHT) {
        xPos = xPos - lineWidth + letterWidth;
    }

    int horOffset = i;

    for (size_t index = 0; text[index] != '\0'; ++index) {
        auto c = text[index];
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
        atlas->transform.pos.x = xPos + ((float)horOffset * (settings.fontSize - trackingWidth));
        i++;
        horOffset++;
        atlas->Update();
    }
}

static Sprite* LoadFont(Platform* platform, const char* pngPath, const char* jsonPath,
                        TEXT_MSDF::FontAtlas& _atlas, Shader* shader, TextureSettings textureSettings) {
    auto texture = platform->CreateTexture(pngPath, textureSettings);
    auto sprite = platform->CreateSprite(texture);
    auto mat = new MaterialFont(shader, texture);
    sprite->SetMaterial(mat);
    std::string data = platform->LoadFileData(jsonPath);
    _atlas = TEXT_MSDF::fromJsonFontAtlas(data.c_str());
    sprite->useAtlas = true;
    sprite->useGlyph = true;
    sprite->atlasWidth = (float)_atlas.atlas.width;
    sprite->atlasHeight = (float)_atlas.atlas.height;
    sprite->transform.width = (float)_atlas.atlas.width * 2.0f / ((float)FRAMEBUFFER_WIDTH*1.0f);
    sprite->transform.height = (float)_atlas.atlas.height * 2.0f / ((float)FRAMEBUFFER_HEIGHT*1.0f);
    sprite->transform.scale = {1, 1, 1};
    sprite->transform.pos = {0,0,0};
    return sprite;
}

static void RenderText(Sprite* _font, const std::string &text, float startX, float startY, const TEXT_MSDF::FontAtlas& _atlas) {
    float x = startX;
    float y = startY;
    //float y = startY + baseline * scale;  // align baseline

    for (char ch : text) {
        TEXT_MSDF::Glyph g{};
        if (ch == '\n') {
            x = startX;
            y -= round(_atlas.atlas.size*_atlas.metrics.lineHeight*_font->transform.scale.y) / FRAMEBUFFER_HEIGHT*2.0f;
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
        _font->glyphxoff = (float)(g.planeBounds.left * _atlas.atlas.size) / (float)FRAMEBUFFER_WIDTH * 2.0f;
        _font->glyphyoff = (float)(g.planeBounds.top * _atlas.atlas.size) / (float)FRAMEBUFFER_HEIGHT *2.0f;
        _font->transform.pos.x = x;
        _font->transform.pos.y = y;
        _font->Update();
        auto pixelAdvance = (float)(g.advance * _atlas.atlas.size * 2.0f);
        pixelAdvance += 2;
        x += (float)(pixelAdvance/(float)FRAMEBUFFER_WIDTH*_font->transform.scale.x*1.0);
    }
}

#endif //GAMEENGINE_TEXT_H
