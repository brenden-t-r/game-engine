#pragma once
#define DEBUG_QUAD 0

#include "engine/game.h"
#include "../engine/platform/directx/directx.h"
#include "model.h"
#include "util.h"

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

        quad = getQuad(platform, vec2{-0.5, -0.5}, {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5});
        tri = (Triangle*)platform->CreateTriangle();
        tri->SetScale({0.02, 0.02, 1.0});


    }

    MaterialColor* mat = nullptr;
    Quad* quad = nullptr;
    Triangle* tri = nullptr;
    bool shouldRotate;
    Tile* hoverTile = new Tile();
    int h = 1;
    int w = 1;
    int l = 1;
    int s = 2;

    void drawDebugTriangle(vec3 pos) {
        auto t = (Triangle*)platform->CreateTriangle();
        t->SetMaterial(mat);
        t->SetPosition(pos);
        t->SetScale({0.01,0.01,1.0});
        t->Update();
        platform->Delete(t);
    }

    void Update() override {
//        mat->color[0]=0.0;
//        mat->color[1]=0.0;
//        mat->color[2]=1.0;
//        mat->color[3]=1.0;
//        quad->SetMaterial(mat);
//        quad->Update();

        SetColor(mat->color, Color{0.0,0.0,0.0,10/255.0});
        drawIsoGrid(platform, mat, 0);
        SetColor(mat->color, COLOR::BLACK);

        auto mousePos = normalized_to_screen(platform->GetMousePos());
//        tri->SetPosition(mousePos);
//        tri->Update();

        if (shouldRotate) {
            // TODO
        }

        bool hover = false;
        bool hoverDrawn = false;

        int row, col;
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

        drawCube(0, 0, 0);

        // TODO: sort tiles
        // TODO: Prune duplicates
        // TODO: Draw tiles to render

        //drawTile(hoverTile);
        if (hover) {
//            printf("Hover: %d %d \n", hoverTile->row, hoverTile->col);
//            drawTile(hoverTile);
        }
    }

    void drawTile(Tile* t) {
        switch(t->type) {
            case CUBE:
                drawCube(t->row, t->col, 0);
                break;
            default:
                assert(false);
        }
    }

    void drawCube(int row, int col, int height) {
        Point bottom[4];
        getIsoTilePoints(row, col, height, bottom);
        Point top[4];
        getIsoTilePoints(row, col, height+1, top);

        auto quadFront = getQuad(platform, bottom[0], top[0], top[3], bottom[3]);
        SetColor(mat->color, COLOR::MID);
        quadFront->SetMaterial(mat);
        quadFront->Update();
        Delete(quadFront);

        auto quadSide = getQuad(platform, bottom[0], bottom[1], top[1], top[0]);
        SetColor(mat->color, COLOR::DARK);
        quadSide->SetMaterial(mat);
        quadSide->Update();
        Delete(quadSide);

        auto quadTop = getQuad(platform, top[0], top[1], top[2], top[3]);
        SetColor(mat->color, COLOR::LIGHTEST);
        quadTop->SetMaterial(mat);
        quadTop->Update();
        Delete(quadTop);

        SetColor(mat->color, COLOR::BLACK);
        drawLine(platform, mat, bottom[0], top[0]);
        drawLine(platform, mat, bottom[1], top[1]);
        drawLine(platform, mat, bottom[3], top[3]);
        drawLine(platform, mat, bottom[0], bottom[1]);
        drawLine(platform, mat, bottom[0], bottom[3]);
        drawLine(platform, mat, top[0], top[1]);
        drawLine(platform, mat, top[1], top[2]);
        drawLine(platform, mat, top[2], top[3]);
        drawLine(platform, mat, top[3], top[0]);
    }

    void MouseReleasedCallback(MouseButton key, vec3 pos) override {
        if (key == MouseButton::Left) {
            printf("mouse click\n");
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
    void Delete(Quad* o) {
        platform->Delete(o->a);
        platform->Delete(o->b);
        delete o;
    }
};