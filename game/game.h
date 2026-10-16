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
    std::vector<Tile*> tiles{};
    Tile* hoverTile = new Tile();
    bool shouldRotate;
    int h = 1;
    int w = 1;
    int l = 1;
    int s = 2;

    void Start() override {
        this->EnableCallback(Callback::KEY_RELEASED);
        this->EnableCallback(Callback::MOUSE_RELEASED);
        platform->LoadShaders();
        auto shader = platform->LoadShader({
            "assets/shaders/color.hlsl","","",InputLayoutType::POSITION
        });
        mat = new MaterialColor(shader);
    }

    void Update() override {
        SetColor(mat->color, Color{0.0,0.0,0.0,10/255.0});
        drawIsoGrid(platform, mat, 0);
        SetColor(mat->color, COLOR::BLACK);

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
};