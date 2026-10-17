#pragma once
#define DEBUG_QUAD 0

#include "engine/game.h"
#include "../engine/platform/directx/directx.h"
#include "model.h"
#include "util.h"

class IsoGame : public Game {
public:
    using Game::Game;
    MaterialColor* mat = nullptr;
    MaterialFont* matFont = nullptr;
    std::vector<Tile*> tiles{};
    Tile* hoverTile = new Tile();
    bool shouldRotate;
    int h = 1;
    int w = 1;
    int l = 1;
    int s = 2;
    Sprite* font;
    TEXT_MSDF::FontAtlas atlas;

    void Start() override {
        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
        platform->LoadShaders();
        auto shader = platform->LoadShader({
            "assets/shaders/color.hlsl","","",InputLayoutType::POSITION
        });
        mat = new MaterialColor(shader);

        auto fontShader = loadFontShader(platform);
        auto linearNoMips = TextureSettings{TextureFilter::LINEAR, false};
        font = loadFont(platform, "arial_msdf", atlas, fontShader, linearNoMips);
        matFont = (MaterialFont*)font->material;
        matFont->color[3] = 0.3;
        matFont->outlineWidth = 0;
        matFont->pxRange = 16;
        matFont->isMSDF = true;
        setFontSize(font, 9);
    }

    void Update() override {
        SetColor(mat->color, Color{0.0,0.0,0.0,10/255.0});
        drawIsoGrid(platform, mat, 0);
        SetColor(mat->color, COLOR::BLACK);

        // Debug display
        auto q = getQuad(platform, Point{20,20},{400,20},{400,200},{20,200});
        SetColor(mat->color, Color(0,0,0,0.05));
        q->SetMaterial(mat);
        q->Update();
        _delete(platform, q);
        SetColor(mat->color, COLOR::BLACK);
        drawLine(platform, mat, Point{20, 20}, Point{400, 20});
        drawLine(platform, mat, Point{20, 20}, Point{20, 200});
        drawLine(platform, mat, Point{20, 200}, Point{400, 200});
        drawLine(platform, mat, Point{400, 20}, Point{400, 200});
        setFontSize(font, 9);
        matFont->color[3] = 1.0;
        drawText(font, atlas, "Selected: Cube", {50, 50});
        drawText(font, atlas, "L: 1  W: 1  H: 1  Orientation: NW", {50, 80});
        matFont->color[3] = 0.3;

        // Rotation
        if (shouldRotate) {
            // TODO
        }

        // Hover preview
        bool hover = false;
        int row, col;
        auto mousePos = normalized_to_screen(platform->GetMousePos());
        if (screenToIsoCell(mousePos.x, mousePos.y, 0, &row, &col)) {
            printf("Hover %d %d\n", row, col);
            hover = true;
            hoverTile->orientation = piece_orientation;
            hoverTile->row = row;
            hoverTile->col = col;
            hoverTile->height_base = 0;
            hoverTile->length = l;
            hoverTile->width = w;
            hoverTile->height = h;
            hoverTile->stairCount = s;
            hoverTile->type = selected;
        }

        // Sort and draw tiles
        drawCube(platform, mat, 0, 0, 0);
        std::sort(tiles.begin(), tiles.end(), compareTiles);
        bool hoverDrawn = false;
        for (auto tile : tiles) {
            if (hover && !hoverDrawn) {
                if (compareTiles(hoverTile, tile)) {
                    hoverDrawn = true;
                    drawTile(hoverTile);
                }
            }
            drawTile(tile);
        }
        if (hover && !hoverDrawn) {
            drawTile(hoverTile);
        }
    }

    void drawTile(Tile* t) {
        switch(t->type) {
            case CUBE:
                drawCube(platform, mat, t->row, t->col, 0);
                break;
            default:
                assert(false);
        }
    }

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        if (key == MouseButton::Left) {
            printf("mouse click\n");
            auto mousePos = normalized_to_screen(platform->GetMousePos());
            int row, col;
            if (screenToIsoCell(mousePos.x, mousePos.y, 0, &row, &col)) {
                if (selected == COMPOSITE_RAMP) {
//                    printf("COMPOSITE RAMP CLICK\n");
//                    auto tcTiles = drawRampCompositeTile(row, col, l, w, h, piece_orientation, -1);
//                    for (auto tile: tcTiles) {
//                        tiles.push_back(tile);
//                    }
                    // TODO:
                } else {
                    Tile *t = nullptr;
                    t = new Tile();
                    t->type = selected,
                    t->orientation = piece_orientation,
                    t->row = row,
                    t->col = col,
                    t->height_base = 0,
                    t->length = l,
                    t->width = w,
                    t->height = h,
                    t->stairCount = s;
                    tiles.push_back(t);
                }
            }
        }
    }
    void KeyReleasedCallback(KeyCode key) override {
        printf("%d\n",key);
        switch (key) {
            case KeyCode::R:
                break;
            case KeyCode::E:
                break;
            case KeyCode::_1:
                break;
            case KeyCode::_2:
                break;
            case KeyCode::_3:
                break;
            case KeyCode::_4:
                break;
            case KeyCode::_5:
                break;
            case KeyCode::L:
                break;
            case KeyCode::W:
                break;
            case KeyCode::H:
                break;
            case KeyCode::S:
                break;
            case KeyCode::Up:
                break;
            case KeyCode::Down:
                break;
            default:
                break;
        }
    }
    void drawIsoGrid(Platform* platform, Material* mat, int height = 0) {
        for (int row = 0; row < GRID_ROWS; row++) {
            for (int col = 0; col < GRID_COLS; col++) {
                int screenX = ORIGIN_X + (col - row) * (TILE_W / 2);
                int screenY = ORIGIN_Y + (col + row) * (TILE_H / 2) - height * TILE_H;
                drawIsoTile(platform, mat, screenX, screenY, TILE_W, TILE_H);
                auto n = screen_to_normalized(vec2{(float)screenX - 20, (float)screenY + TILE_H/2 + 6});
                char buffer[100];
                sprintf(buffer, "(%d,%d)", row, col);
                RenderText(font, buffer, n.x, n.y, atlas);
            }
        }
    }
};