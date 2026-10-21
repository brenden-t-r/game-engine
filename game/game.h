#pragma once
#define DEBUG_QUAD 0
#define DEBUG_PRINT 0

#include "../engine/engine/game.h"
#include "../engine/platform/directx/directx.h"

#include "model.h"
#include "util.h"
#include "ramp.h"

class IsoGame : public Game {
public:
    using Game::Game;

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
        //region: Init
        frame += 1;
#if DEBUG_PRINT
        printf("frame: %d\n", frame);
#endif
        if (platform->IsKeyPressed(KeyCode::Right)) {
            OffsetX += 1;
        }
        SetColor(mat->color, Color{0.0,0.0,0.0,10/255.0});
        drawIsoGrid(platform, mat, 0);
        SetColor(mat->color, COLOR::BLACK);
        //endregion

        // region: Debug display
        auto q = getQuad(platform, Point{20,20},{400,20},{400,200},{20,200});
        SetColor(mat->color, Color(0,0,0,0.05));
        q->SetMaterial(mat);
        q->Update();
        deleteQuad(platform, q);
        SetColor(mat->color, COLOR::BLACK);
        drawLine(platform, mat, Point{20, 20}, Point{400, 20});
        drawLine(platform, mat, Point{20, 20}, Point{20, 200});
        drawLine(platform, mat, Point{20, 200}, Point{400, 200});
        drawLine(platform, mat, Point{400, 20}, Point{400, 200});
        setFontSize(font, 9);
        matFont->color[3] = 1.0;
        char buffer[100];
        sprintf(buffer, "Selected: %s", selectedPieceStr());
        drawText(font, atlas, buffer, {50, 50});
        sprintf(buffer, "L: %d  W: %d  H: %d  Global: %s Piece: %s", l, w, h,
                orientationToStr(selected_orientation), orientationToStr(piece_orientation));
        drawText(font, atlas, buffer, {50, 80});
        matFont->color[3] = 0.3;
        //endregion

        //region: Rotation
        if (shouldRotate) {
            shouldRotate = false;
            switch(selected_orientation) {
                case NORTHEAST: selected_orientation = SOUTHEAST;
                    break;
                case SOUTHEAST: selected_orientation = SOUTHWEST;
                    break;
                case SOUTHWEST: selected_orientation = NORTHWEST;
                    break;
                case NORTHWEST: selected_orientation = NORTHEAST;
                    break;
            }
            for (int i = 0; i < tiles.size(); i++){
                if (tiles[i]->type != COMPOSITE_RAMP_UNIT) {
                    int row = tiles[i]->row;
                    int col = tiles[i]->col;
                    int rotatedRow = col;
                    int rotatedCol = GRID_ROWS - 1 - row;
                    tiles[i]->row = rotatedRow;
                    tiles[i]->col = rotatedCol;
                }
            }
        }
        //endregion

        //region: Hover preview
        bool hover = false;
        int row, col;
        auto mousePos = normalized_to_screen(platform->GetMousePos());
        bool canDraw = true;
        int hoverBaseRow = -1;
        int hoverBaseCol = -1;
        if (screenToIsoCell(mousePos.x, mousePos.y, 0, &row, &col)) {
            hover = true;

            for (auto t : tiles) {
                if (t->row == row && t->col==col) {
                    canDraw = false;
                }
            }

            if (selected == COMPOSITE_RAMP) {
                auto _tiles = drawRampCompositeTile(platform, mat, row, col, l, w, h, piece_orientation);
                auto p = new CompositePiece();
                for (auto t : _tiles) {
                    p->tiles.push_back(t);
                }
                for (auto t : tiles) {
                    for (auto thisT : _tiles) {
                        if (thisT->row == t->row && thisT->col == t->col) {
                            // todo effective row w height
                            canDraw = false;
                        }
                    }
                }
                hoverTile = p;
                if (piece_orientation == SOUTHWEST) {
                    hoverBaseRow = row + l - 1;
                    hoverBaseCol = col;
                } else if (piece_orientation == SOUTHEAST) {
                    hoverBaseCol = col + l - 1;
                    hoverBaseRow = row;
                }
            } else {
                hoverTile = hoverBasic;
            }
            hoverTile->orientation = piece_orientation;
            hoverTile->row = row;
            hoverTile->col = col;
            hoverTile->height_base = 0;
            hoverTile->length = l - 1;
            hoverTile->width = w - 1;
            hoverTile->height = 0; // hardcoded for now
            hoverTile->stairCount = s;
            hoverTile->type = selected;
        }
        //endregion

        //region: Sort and draw tiles
        std::sort(tiles.begin(), tiles.end(), compareTiles);
        bool hoverDrawn = false;
        for (auto tile : tiles) {
            if (hover && !hoverDrawn) {
                if (hoverBaseRow>-1) {
                    Tile* tmp = new Tile();
                    tmp->row = hoverBaseRow;
                    tmp->col = hoverBaseCol;
                    tmp->height = 0;
                    if (compareTiles(tmp, tile)) {
                        hoverDrawn = true;
                        if (canDraw) {
                            drawTile(hoverTile);
                        }
                    }
                    delete tmp;
                } else {
                    if (compareTiles(hoverTile, tile)) {
                        hoverDrawn = true;
                        if (canDraw) {
                            drawTile(hoverTile);
                        }
                    }
                }
            }
            drawTile(tile);
        }
        if (hover && !hoverDrawn && canDraw) {
            drawTile(hoverTile);
        }
        if (hoverDrawn && hoverTile->type == COMPOSITE_RAMP) {
            delete(hoverTile);
        }
        //endregion
    }

private:
    MaterialColor* mat = nullptr;
    MaterialFont* matFont = nullptr;
    std::vector<Tile*> tiles{};
    Tile* hoverTile = new Tile();
    Tile* hoverBasic = new Tile();
    bool shouldRotate;
    int h = 1;
    int w = 1;
    int l = 1;
    int s = 2;
    Sprite* font;
    TEXT_MSDF::FontAtlas atlas;
    int frame = 0;

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        if (key == MouseButton::Left) {
            auto mousePos = normalized_to_screen(platform->GetMousePos());
            int row, col;
            if (screenToIsoCell(mousePos.x, mousePos.y, 0, &row, &col)) {
                if (selected == COMPOSITE_RAMP) {
                    auto tcTiles = drawRampCompositeTile(platform, mat, row, col, l, w, h, piece_orientation);
                    for (auto tile: tcTiles) {
                        tiles.push_back(tile);
                    }
                } else {
                    Tile *t = nullptr;
                    t = new Tile();
                    t->type = selected,
                    t->orientation = piece_orientation,
                    t->row = row,
                    t->col = col,
                    t->height_base = 0,
                    t->length = l - 1,
                    t->width = w - 1,
                    t->height = 0, // TODO: Support height and/or base height for cube etc.
                    t->stairCount = s;
                    tiles.push_back(t);
                }
            }
        }
    }
    void KeyReleasedCallback(KeyCode key) override {
        switch (key) {
            case KeyCode::R:
                shouldRotate = true;
                break;
            case KeyCode::E:
                switch(piece_orientation) {
                    case NORTHEAST: piece_orientation = SOUTHEAST;
                        break;
                    case SOUTHEAST: piece_orientation = SOUTHWEST;
                        break;
                    case SOUTHWEST: piece_orientation = NORTHWEST;
                        break;
                    case NORTHWEST: piece_orientation = NORTHEAST;
                        break;
                }
                break;
            case KeyCode::_1:
                selected = CUBE;
                break;
            case KeyCode::_2:
                break;
            case KeyCode::_3:
                break;
            case KeyCode::_4:
                break;
            case KeyCode::_5:
                selected = COMPOSITE_RAMP;
                break;
            case KeyCode::L:
                selected_tool = LENGTH;
                break;
            case KeyCode::W:
                selected_tool = WIDTH;
                break;
            case KeyCode::H:
                selected_tool = HEIGHT;
                break;
            case KeyCode::S:
                break;
            case KeyCode::Up:
                switch (selected_tool) {
                    case WIDTH: w++; break;
                    case LENGTH: l++; break;
                    case HEIGHT: h++; break;
                    case STAIRCOUNT: s++; break;
                }
                break;
            case KeyCode::Down:
                switch (selected_tool) {
                    case WIDTH: w--; break;
                    case LENGTH: l--; break;
                    case HEIGHT: h--; break;
                    case STAIRCOUNT: s--; break;
                }
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
    void drawTile(Tile* t) {
#if DEBUG_PRINT
        printTile(t);
#endif
        switch(t->type) {
            case CUBE:
                drawCube(platform, mat, t->row, t->col, 0);
                break;
            case COMPOSITE_RAMP: {
                // Used for hover display only
                auto cr = dynamic_cast<CompositePiece*>(t);
                for (auto tile: cr->tiles) {
                    drawTile(tile);
                }
                break;
            }
            case COMPOSITE_RAMP_UNIT: {
                auto cruTile = dynamic_cast<CompositeTile *>(t);
                for (auto face: cruTile->faces) {
                    auto points = face.points;
                    if (points.size() == 3) {
                        auto tri = getTriangle(platform, points[0], points[1], points[2]);
                        SetColor(mat->color, face.color);
                        tri->SetMaterial(mat);
                        tri->Update();
                        platform->Delete(tri);
                    } else if (points.size() == 4) {
                        auto q1 = getQuad(platform, points[0], points[1], points[2], points[3]);
                        drawQuadAndCleanUp(platform, mat, q1, face.color);
                    } else if (points.size() == 5) {
                        auto q1 = getQuad(platform, points[0], points[1], points[2], points[3]);
                        auto q2 = getQuad(platform, points[1], points[2], points[3], points[4]);
                        auto q3 = getQuad(platform, points[0], points[1], points[3], points[4]);
                        auto q4 = getQuad(platform, points[0], points[1], points[2], points[4]);
                        auto q5 = getQuad(platform, points[0], points[2], points[3], points[4]);
                        drawQuadAndCleanUp(platform, mat, q1, face.color);
                        drawQuadAndCleanUp(platform, mat, q2, face.color);
                        drawQuadAndCleanUp(platform, mat, q3, face.color);
                        drawQuadAndCleanUp(platform, mat, q4, face.color);
                        drawQuadAndCleanUp(platform, mat, q5, face.color);
                    }
                }
                SetColor(mat->color, COLOR::BLACK);
                for (auto line: cruTile->lines) {
                    drawLine(platform, mat, line.a, line.b);
                }
                break;
            }
            default:
                assert(false);
        }
    }
};