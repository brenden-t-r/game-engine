#pragma once
#define DEBUG_QUAD 0
#define DEBUG_PRINT 0

#include "../engine/engine/game.h"
#include "../engine/platform/directx/directx.h"

#include "model.h"
#include "util.h"
#include "ramp.h"
#include "cube.h"

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

        CenterCameraOnCell(0,0,0);
    }

    void Update() override {
        //region: Init
        frame += 1;
#if DEBUG_PRINT
        printf("frame: %d\n", frame);
#endif
        if (platform->IsKeyPressed(KeyCode::D)) {
            CAMERA.x += 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::A)) {
            CAMERA.x -= 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::W)) {
            CAMERA.y += 0.01;
        }
        if (platform->IsKeyPressed(KeyCode::S)) {
            CAMERA.y -= 0.01;
        }
        SetColor(mat->color, Color{0.0,0.0,0.0,10/255.0});
        drawIsoGrid(platform, mat, 0);
        SetColor(mat->color, COLOR::BLACK);

        int ccol = GetCenterCol(0);
        int crow = GetCenterRow(0);
//        printf("%d %d\n", crow, ccol);
        fflush(stdout);
        //endregion

        // region: Debug display
        auto q = getQuad(platform, Point{20,20},{400,20},{400,200},{20,200});
        q->a->overlay = true;
        q->b->overlay = true;
        SetColor(mat->color, Color(0,0,0,0.05));
        q->SetMaterial(mat);
        q->Update();
        deleteQuad(platform, q);
        SetColor(mat->color, COLOR::BLACK);
        font->overlay = true;
        setFontSize(font, 9);
        matFont->color[3] = 1.0;
        char buffer[100];
        sprintf(buffer, "Selected: %s", selectedPieceStr());
        drawText(font, atlas, buffer, {50, 50});
        sprintf(buffer, "L: %d  W: %d  H: %d  Global: %s Piece: %s", l, w, h,
                orientationToStr(selected_orientation), orientationToStr(piece_orientation));
        drawText(font, atlas, buffer, {50, 80});
        matFont->color[3] = 0.3;
        font->overlay = false;
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
            std::vector<Tile*> temp = std::move(tiles);
            tiles = {};
            std::vector<int> seen{};

            for (int i = 0; i < temp.size(); i++){
                auto t = temp[i];
                if (t->type != COMPOSITE_RAMP_UNIT && t->type != COMPOSITE_RAMP) {
                    int row = t->row;
                    int col = t->col;
                    int rotatedRow = col;
                    int rotatedCol = -row;
//                    int rotatedRow = col;
//                    int rotatedCol = GRID_ROWS - 1 - row;
                    t->row = rotatedRow;
                    t->col = rotatedCol;
                    tiles.push_back(t);
                } else {
                    auto parent = ((CompositeTile*)t)->parent;
                    bool hasSeen = false;
                    for (auto id : seen) {
                        if (parent->id==id) {
                            hasSeen = true;
                            break;
                        }
                    }
                    if (!hasSeen) {
                        seen.push_back(parent->id);
                        int rotatedRow = parent->col;
                        int rotatedCol = -parent->row;

                        auto rotated = drawRampCompositeTile(
                                platform, mat,
                                rotatedRow, rotatedCol, parent->length, parent->width, parent->height,
                                rotatedOrientation(parent->orientation)
                        );
                        for (auto tile : rotated) {
                            tiles.push_back(tile);
                        }
                    }
                }
            }

            // Update cam
            int rotated_crow = ccol;
            int rotated_ccol = -crow;
            CenterCameraOnCell(rotated_crow, rotated_ccol, 0);
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
                auto p = _tiles[0]->parent;
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
            } else if (selected == COMPOSITE_CUBE) {
                auto p = drawCubeCompositeTile(platform, mat, row, col, l, w, h, piece_orientation);
                for (auto t : tiles) {
                    for (auto child : p->tiles) {
                        if (child->row == t->row && child->col == t->col) {
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
        bool playerDrawn = false;
        Tile* camTile = new Tile();
        camTile->row = GetCenterRow(0);
        camTile->col = GetCenterCol(0);
        camTile->height = 0;
        for (auto tile : tiles) {
            if (tile->type == COMPOSITE_RAMP_UNIT || tile->type == COMPOSITE_CUBE_UNIT) {
                auto p = ((CompositeTile*)tile)->parent;
                bool occluding = false;
                for (auto child : p->tiles) {
                    int r = child->row;
                    int c = child->col;
                    r -= child->height;
                    c -= child->height;
                    if (r == camTile->row && c == camTile->col) {
                        occluding = true;
                        break;
                    } else if (r - 1 == camTile->row && c - 1 == camTile->col) {
                        occluding = true;
                        break;
                    }
                }
                p->occluding = occluding;
            }

            if (!playerDrawn) {
                if (compareTiles(camTile, tile)) {
                    playerDrawn = true;
                    Triangle* tri = (Triangle*)platform->CreateTriangle();
                    tri->SetPosition({CAMERA.x, CAMERA.y});
                    tri->SetScale({0.1,0.1,1});
                    tri->Update();
                    platform->Delete(tri);
                }
            }

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
        delete camTile;

        if (!playerDrawn) {
            Triangle* tri = (Triangle*)platform->CreateTriangle();
            tri->SetPosition({CAMERA.x, CAMERA.y});
            tri->manualVertices = false;
            tri->overlay = false;
            tri->SetScale({0.1, 0.1,1});
            tri->Update();
            platform->Delete(tri);
        }
        if (hover && !hoverDrawn && canDraw) {
            drawTile(hoverTile);
        }
        if (hoverDrawn && (hoverTile->type == COMPOSITE_RAMP || hoverTile->type == COMPOSITE_CUBE)) {
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
    bool shouldRotate = false;
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
                    auto p = tcTiles[0]->parent;
                    for (auto tile: tcTiles) {
                        tiles.push_back(tile);
                        p->tiles.push_back(tile);
                    }
                } else if (selected == COMPOSITE_CUBE) {
                    auto p = drawCubeCompositeTile(platform, mat, row, col, l, w, h, piece_orientation);
                    for (auto tile : p->tiles) {
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
                selected = COMPOSITE_CUBE;
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

    void CenterCameraOnCell(int row, int col, int height) {
//        printf("Center on %d %d\n", row, col);
        float x = ORIGIN_X + (col - row) * (TILE_W / 2.0f);
        float y = ORIGIN_Y + (col + row) * (TILE_H / 2.0f)
                  - height * TILE_H;
        y += TILE_H/2;

        auto n = screen_to_normalized(vec2{x, y});
        CAMERA.x = n.x;
        CAMERA.y = n.y;
    }
    int GetCenterCol(int height) {
        auto cam = normalized_to_screen(vec2{CAMERA.x, CAMERA.y});

        return (int)floorf(
                (cam.x - ORIGIN_X) / TILE_W
                + (cam.y - ORIGIN_Y + height * TILE_H) / TILE_H
        );
    }
    int GetCenterRow(int height) {
        auto cam = normalized_to_screen(vec2{CAMERA.x, CAMERA.y});

        return (int)floorf(
                (cam.y - ORIGIN_Y + height * TILE_H) / TILE_H
                - (cam.x - ORIGIN_X) / TILE_W
        );
    }
    void drawIsoGrid(Platform* platform, Material* mat, int height = 0) {
        std::vector<vec2> lines{};
        for (int row = -GRID_ROWS; row < GRID_ROWS; row++) {
            for (int col = -GRID_COLS; col < GRID_COLS; col++) {
                int screenX = ORIGIN_X + (col - row) * (TILE_W / 2);
                int screenY = ORIGIN_Y + (col + row) * (TILE_H / 2) - height * TILE_H;

                vec2 points[5] = {
                        screen_to_normalized(vec2{ (float)screenX,              (float)screenY }),
                        screen_to_normalized(vec2{ (float)(screenX + TILE_W/2),  (float)(screenY + TILE_H/2)  }),
                        screen_to_normalized(vec2{ (float)screenX,              (float)(screenY + TILE_H)     }),
                        screen_to_normalized(vec2{ (float)(screenX - TILE_W/2),  (float)(screenY + TILE_H/2)  }),
                        screen_to_normalized(vec2{ (float)screenX,              (float)screenY            }),
                };
                lines.push_back(points[0]);  lines.push_back(points[1]);
                lines.push_back(points[1]);  lines.push_back(points[2]);
                lines.push_back(points[2]);  lines.push_back(points[3]);
                lines.push_back(points[3]);  lines.push_back(points[4]);

                auto n = screen_to_normalized(vec2{(float)screenX - 20, (float)screenY + TILE_H/2 + 6});
                char buffer[100];
                sprintf(buffer, "(%d,%d)", row, col);
//                RenderText(font, buffer, n.x, n.y, atlas);
            }
        }
       ((PlatformDirectX*)platform)->drawLines(lines, mat);
    }
    void drawTile(Tile* t) {
#if DEBUG_PRINT
        printTile(t);
#endif
        switch(t->type) {
            case CUBE:
                drawCube(platform, mat, t->row, t->col, 0);
                break;
            case COMPOSITE_CUBE: {
                // Used for hover display only
                auto cr = dynamic_cast<CompositePiece*>(t);
                std::sort(cr->tiles.begin(), cr->tiles.end(), compareTiles);
                for (auto tile: cr->tiles) {
                    drawTile(tile);
                }
                break;
            }
            case COMPOSITE_CUBE_UNIT: {
                auto ccuTile = dynamic_cast<CompositeTile *>(t);
                for (auto face: ccuTile->faces) {
                    if (ccuTile->parent->occluding) face.color.a = 0.0;
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
                    }
                }
                SetColor(mat->color, COLOR::BLACK);
                for (auto line: ccuTile->lines) {
                    drawLine(platform, mat, line.a, line.b);
                }
                break;
            }
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
                    if (cruTile->parent->occluding) face.color.a = 0.0;
                    else face.color.a = 1.0;
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