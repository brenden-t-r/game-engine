#include "../engine/constants.h"
#include "../engine/engine/gameobject.h"

#include <vector>

#pragma once

const int TILE_W = 64*2;
const int TILE_H = 32*2;
const int GRID_COLS = 10;
const int GRID_ROWS = 10;
const int ORIGIN_X = WINDOW_WIDTH / 2;
const int ORIGIN_Y = WINDOW_HEIGHT/ 4;
int OffsetX = 0;
int OffsetY = 0;

enum TOOL {
    WIDTH, LENGTH, HEIGHT, STAIRCOUNT
};
enum PIECE_TYPE {
    CUBE, RAMP, STAIRS,
    COMPOSITE_CUBE, COMPOSITE_RAMP, COMPOSITE_RAMP_UNIT
};
PIECE_TYPE selected = CUBE;
TOOL selected_tool = WIDTH;
enum Orientation {
    NORTHEAST, NORTHWEST, SOUTHWEST, SOUTHEAST
};
Orientation selected_orientation = NORTHEAST;
Orientation piece_orientation = NORTHEAST;
Orientation getOrientation(Orientation global, Orientation object)
{
    return static_cast<Orientation>((global + object) % 4);
}
struct Vec2 { float x, y; };
struct Point {
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}

    int x, y;
    bool operator<(const Point& other) const
    {
        if (x != other.x)
            return x < other.x;

        return y < other.y;
    }
};
static int ID = 0;
struct Tile {
    Tile(){
        id = ++ID;
//        printf("Create tile %d\n", id);
    }
    virtual ~Tile() = default;
    PIECE_TYPE type;
    Orientation orientation;
    int row;
    int col;
    int height_base;
    int length;
    int width;
    int height;
    int stairCount;
    int id;
};
enum class COLOR {
    BLACK, DARK, MID, MID2, LIGHTEST
};
class Color {
public:
    float r,g,b,a;
    Color(float r, float g, float b, float a): r(r), g(g), b(b), a(a){}
};
static Color GetColor(COLOR c) {
    switch (c) {
        case COLOR::BLACK: return Color(0.0, 0.0, 0.0, 1.0);
        case COLOR::DARK: return Color(60/255.0, 60/255.0, 60/255.0, 1.0);
        case COLOR::MID: return Color(150/255.0, 150/255.0, 150/255.0, 1.0);
        case COLOR::MID2: return Color(180/255.0, 180/255.0, 180/255.0, 1.0);
        case COLOR::LIGHTEST: return Color(250/255.0, 250/255.0, 250/255.0, 1.0);
    }
}
static void SetColor(float* float4, COLOR c) {
    auto color = GetColor(c);
    float4[0] = color.r;
    float4[1] = color.g;
    float4[2] = color.b;
    float4[3] = color.a;
}
static void SetColor(float* float4, Color c) {
    float4[0] = c.r;
    float4[1] = c.g;
    float4[2] = c.b;
    float4[3] = c.a;
}
struct CompositeTileFace{
    std::vector<Point>points;
    Color color;
};
struct CompositeTileLine{
    Point a;
    Point b;
};
struct CompositePiece : public Tile{
    std::vector<Tile*>tiles;
};
struct CompositeTile : public Tile{
    std::vector<CompositeTileFace>faces{};
    std::vector<CompositeTileLine>lines{};
    CompositePiece* parent = nullptr;
};

class Quad {
public:
    Triangle* a = nullptr;
    Triangle* b = nullptr;
    vec2 points[4];
    void SetMaterial(MaterialColor* mat) {
        a->material = mat;
        b->material = mat;
    }
    void Update() {
#if DEBUG_QUAD
        ((MaterialColor*)a->material)->color[0] = 0.0;
#endif
        a->Update();
#if DEBUG_QUAD
        ((MaterialColor*)b->material)->color[0] = 1.0;
#endif
        b->Update();
    }
};