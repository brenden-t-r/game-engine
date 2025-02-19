#ifndef GAMEENGINE_TEXT_H
#define GAMEENGINE_TEXT_H

#include "vector.h"

enum ParagraphAlignment{
    LEFT, MIDDLE, RIGHT
};
struct ParagraphSettings{
    ParagraphAlignment alignment;
    float fontSize;
    Vector3 pos;
    float tracking;
};

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

#endif //GAMEENGINE_TEXT_H
