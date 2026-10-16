#include "../engine/engine/matrix.h"
#include "../engine/platform/platform.h"
#include "../engine/platform/directx/directx.h"

#include "model.h"

#include <cmath>
#include <algorithm>

#pragma once

//region: math
float distance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}
float distance(Point p1, Point p2)
{
    return distance((float)p1.x, (float)p1.y, (float)p2.x, (float)p2.y);
}
Vec2 multiplyVectorPoint(Point p, Vec2 dirNormalized, float dist)
{
    return { p.x + dirNormalized.x * dist, p.y + dirNormalized.y * dist };
}
Vec2 getBasisVector(Orientation orientation) {
    float length = sqrt((float)(TILE_W*TILE_W + TILE_H*TILE_H));
    switch(orientation){
        case NORTHEAST:
            return Vec2{(float)TILE_W/length, -(float)TILE_H/length};
        case NORTHWEST:
            return Vec2{-(float)TILE_W/length, -(float)TILE_H/length};
        case SOUTHWEST:
            return Vec2{-(float)TILE_W/length, (float)TILE_H/length};
        case SOUTHEAST:
            return Vec2{(float)TILE_W/length, (float)TILE_H/length};
    }
}
void getIsoTilePointsF(float row, float col, float height, Point points[4]) {
    float screenX = ORIGIN_X + (col - row) * (TILE_W / 2.0f) + OffsetX;
    float screenY = ORIGIN_Y + (col + row) * (TILE_H / 2.0f) - height * TILE_H + OffsetY;
    points[0] = Point(screenX, screenY + TILE_H);           // bottom
    points[1] = Point(screenX + TILE_W / 2.0f, screenY + TILE_H / 2.0f);  // right
    points[2] = Point(screenX, screenY);                    // top
    points[3] = Point(screenX - TILE_W / 2.0f, screenY + TILE_H / 2.0f);  // left
}

Vec2 getScreenCordsForTileLocation(float row, float col, float height) {
    float screenX = ORIGIN_X + (col - row) * (TILE_W / 2.0f) + OffsetX;
    float screenY = ORIGIN_Y + (col + row) * (TILE_H / 2.0f) - height * TILE_H + OffsetY;
    return Vec2{screenX, screenY};
}

void getCompositeTilePointsF(float row, float col, float height,
                             float rowOffset, float heightOffset,
                             Point points[8]) {
    float screenX = ORIGIN_X + (col - row) * (TILE_W / 2.0f) + OffsetX;
    float screenY = ORIGIN_Y + (col + row) * (TILE_H / 2.0f) - height * TILE_H + OffsetY;
    points[0] = Point(screenX - (rowOffset*(TILE_W / 2.0f)), screenY + TILE_H + (rowOffset*(TILE_H / 2.0f)));           // bottom
    points[1] = Point(screenX + TILE_W / 2.0f, screenY + TILE_H / 2.0f - (heightOffset*TILE_H));  // right
    points[2] = Point(screenX, screenY + rowOffset - (heightOffset*TILE_H));                    // top
    points[3] = Point(screenX - TILE_W / 2.0f  - (rowOffset*(TILE_W / 2.0f)), screenY + TILE_H / 2.0f  + (rowOffset*(TILE_H / 2.0f)));  // left
}

void getIsoTilePoints(int row, int col, int height, Point points[4]) {
    int screenX = ORIGIN_X + (col - row) * (TILE_W / 2) + OffsetX;
    int screenY = ORIGIN_Y + (col + row) * (TILE_H / 2) - height * TILE_H + OffsetY;
    points[0] = Point(screenX, screenY + TILE_H);           // bottom
    points[1] = Point(screenX + TILE_W / 2, screenY + TILE_H / 2);  // right
    points[2] = Point(screenX, screenY);                    // top
    points[3] = Point(screenX - TILE_W / 2, screenY + TILE_H / 2);  // left
}
bool screenToIsoCell(float sx, float sy,int height,int* outRow, int* outCol)
{
    // undo origin + height offset
    float x = sx - ORIGIN_X;
    float y = sy - ORIGIN_Y + height * TILE_H;

    float halfW = TILE_W * 0.5f;
    float halfH = TILE_H * 0.5f;

    // invert the transform
    float colf = (x / halfW + y / halfH) * 0.5f;
    float rowf = (y / halfH - x / halfW) * 0.5f;

    // candidate cell
    int col = (int)floorf(colf);
    int row = (int)floorf(rowf);

    // bounds check
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS)
        return false;

    *outRow = row;
    *outCol = col;
    return true;
}
int tileSortKey(const Tile* t) {
    return ((t->row+t->height) + (t->col+t->height)); // - 2 * t.height;
}
bool compareTiles(const Tile* a, const Tile* b) {
    printf("%d => (%d,%d) vs %d => (%d,%d)\n", a->id, a->row, a->col, b->id, b->row, b->col);
    int ka = tileSortKey(a);
    int kb = tileSortKey(b);
    if (ka != kb)return ka < kb;
    if (a->row != b->row)return a->row < b->row;
    if (a->col != b->col)return a->col < b->col;
    else {
        if (auto aCast = dynamic_cast<const CompositeTile*>(a)) {
            if (auto bCast = dynamic_cast<const CompositeTile*>(b)) {
                auto aParent = aCast->parent;
                auto bParent = bCast->parent;
                if (aParent == nullptr || bParent == nullptr) {
                    printf("Nullptr parent\n");
                    return a->col < b->col;
                }
                printf("%d(A) and %d(B) parents %d %d , %d %d\n", a->id, b->id, aParent->row, aParent->col, bParent->row, bParent->col);
                return compareTiles(aParent, bParent);
            } else {
                return a->col < b->col;
            }
        } else {
            return a->col < b->col;
        }
    }
}
bool LineQuadIntersection(Point lineA, Point lineB,Point quad[4], Point& outA, Point& outB)
{
    auto cross = [](Point a, Point b)
    {
        return a.x * b.y - a.y * b.x;
    };

    auto samePoint = [](Point a, Point b)
    {
        return std::abs(a.x - b.x) < 0.00001f &&
               std::abs(a.y - b.y) < 0.00001f;
    };

    Point r{
            lineB.x - lineA.x,
            lineB.y - lineA.y
    };

    Point intersections[4];
    int count = 0;

    for (int i = 0; i < 4; ++i)
    {
        Point p = quad[i];

        Point s{
                quad[(i + 1) % 4].x - p.x,
                quad[(i + 1) % 4].y - p.y
        };

        float denom = cross(r, s);

        if (std::abs(denom) < 0.00001f)
            continue;

        Point qp{
                p.x - lineA.x,
                p.y - lineA.y
        };

        float t = cross(qp, s) / denom;
        float u = cross(qp, r) / denom;

        if (u >= -0.00001f && u <= 1.00001f)
        {
            Point intersection{
                    static_cast<int>(lineA.x + t * r.x),
                    static_cast<int>(lineA.y + t * r.y)
            };

            // Don't add the same vertex twice.
            bool duplicate = false;

            for (int j = 0; j < count; ++j)
            {
                if (samePoint(intersection, intersections[j]))
                {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate)
                intersections[count++] = intersection;
        }
    }

    if (count != 2)
        return false;

    outA = intersections[0];
    outB = intersections[1];

    return true;
}
bool isAboveLine(Point p1, Point p2, Point p3)
{
    float cross =
            (p2.x - p1.x) * (p3.y - p1.y) -
            (p2.y - p1.y) * (p3.x - p1.x);

    // In screen coordinates (Y increases downward),
    // positive means the point is above the directed line.
    return cross < 0;
}
bool lineSegmentIntersection(
        Point pa1, Point pa2,   // Infinite line
        Point pb1, Point pb2,   // Finite segment
        Point& intersection)
{
    float dxA = pa2.x - pa1.x;
    float dyA = pa2.y - pa1.y;

    float dxB = pb2.x - pb1.x;
    float dyB = pb2.y - pb1.y;

    float denominator = dxA * dyB - dyA * dxB;

    // Parallel lines
    if (denominator == 0.0f)
        return false;

    float dx = pb1.x - pa1.x;
    float dy = pb1.y - pa1.y;

    // Position along infinite line A
    float t = (dx * dyB - dy * dxB) / denominator;

    // Position along segment B
    float u = (dx * dyA - dy * dxA) / denominator;

    // Intersection must be within segment B
    if (u < 0.0f || u > 1.0f)
        return false;

    intersection.x = pa1.x + t * dxA;
    intersection.y = pa1.y + t * dyA;

    return true;
}
bool addUnique(std::vector<Point>& points, Point p)
{
    for (const Point& existing : points)
    {
        if (existing.x == p.x && existing.y == p.y)
            return false; // Already exists
    }

    points.push_back(p);
    return true;
}
void sortPointsAroundCenter(std::vector<Point>& points)
{
    float centerX = 0.0f;
    float centerY = 0.0f;

    for (const Point& p : points)
    {
        centerX += p.x;
        centerY += p.y;
    }

    centerX /= points.size();
    centerY /= points.size();

    std::sort(points.begin(), points.end(),
              [centerX, centerY](const Point& a, const Point& b)
              {
                  float angleA = std::atan2(a.y - centerY, a.x - centerX);
                  float angleB = std::atan2(b.y - centerY, b.x - centerX);

                  return angleA < angleB;
              });
}
//endregion

//region: draw
void drawIsoTile(Platform* platform, Material* mat, int screenX, int screenY, int tileW, int tileH) {
    vec2 points[5] = {
            screen_to_normalized(vec2{ (float)screenX,              (float)screenY }),
            screen_to_normalized(vec2{ (float)(screenX + tileW/2),  (float)(screenY + tileH/2)  }),
            screen_to_normalized(vec2{ (float)screenX,              (float)(screenY + tileH)     }),
            screen_to_normalized(vec2{ (float)(screenX - tileW/2),  (float)(screenY + tileH/2)  }),
            screen_to_normalized(vec2{ (float)screenX,              (float)screenY            }),
    };
    ((PlatformDirectX*)platform)->drawLine(points[0], points[1], mat);
    ((PlatformDirectX*)platform)->drawLine(points[1], points[2], mat);
    ((PlatformDirectX*)platform)->drawLine(points[2], points[3], mat);
    ((PlatformDirectX*)platform)->drawLine(points[3], points[4], mat);

}
void drawIsoGrid(Platform* platform, Material* mat, int height = 0) {
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            int screenX = ORIGIN_X + (col - row) * (TILE_W / 2);
            int screenY = ORIGIN_Y + (col + row) * (TILE_H / 2) - height * TILE_H;
            drawIsoTile(platform, mat, screenX, screenY, TILE_W, TILE_H);
        }
    }
}
Triangle* getTriangle(Platform* platform, vec2 _1, vec2 _2, vec2 _3) {
    auto t = (Triangle*)platform->CreateTriangle();
    t->manualVertices = true;
    t->vertices[0] = {_1.x, _1.y, 0.0};
    t->vertices[1] = {_2.x, _2.y, 0.0};
    t->vertices[2] = {_3.x, _3.y, 0.0};
    return t;
}
vec2 pointToVec2(Point p){
    return screen_to_normalized(vec2{(float)p.x, (float)p.y});
}
Quad* getQuad(Platform* platform, Point _1, Point _2, Point _3, Point _4) {
    auto o = new Quad();
    o->a = getTriangle(platform, pointToVec2(_1), pointToVec2(_2), pointToVec2(_3));
    o->b = getTriangle(platform, pointToVec2(_1), pointToVec2(_3), pointToVec2(_4));
    return o;
}
Quad* getQuad(Platform* platform, vec2 _1, vec2 _2, vec2 _3, vec2 _4) {
    auto o = new Quad();
    o->a = getTriangle(platform, _1, _2, _3);
    o->b = getTriangle(platform, _1, _3, _4);
    return o;
}
void drawLine(Platform* platform, Material* mat, Point a, Point b) {
    ((PlatformDirectX*)platform)->drawLine(pointToVec2(a), pointToVec2(b), mat);
}
void drawDebugTriangle(Platform* platform, Material* mat, vec3 pos) {
    auto t = (Triangle*)platform->CreateTriangle();
    t->SetMaterial(mat);
    t->SetPosition(pos);
    t->SetScale({0.01,0.01,1.0});
    t->Update();
    platform->Delete(t);
}
void _delete(Platform* platform, Quad* o) {
    platform->Delete(o->a);
    platform->Delete(o->b);
    delete o;
}
void drawCube(Platform* platform, MaterialColor* mat, int row, int col, int height) {
    Point bottom[4];
    getIsoTilePoints(row, col, height, bottom);
    Point top[4];
    getIsoTilePoints(row, col, height+1, top);

    auto quadFront = getQuad(platform, bottom[0], top[0], top[3], bottom[3]);
    SetColor(mat->color, COLOR::MID);
    quadFront->SetMaterial(mat);
    quadFront->Update();
    _delete(platform, quadFront);

    auto quadSide = getQuad(platform, bottom[0], bottom[1], top[1], top[0]);
    SetColor(mat->color, COLOR::DARK);
    quadSide->SetMaterial(mat);
    quadSide->Update();
    _delete(platform, quadSide);

    auto quadTop = getQuad(platform, top[0], top[1], top[2], top[3]);
    SetColor(mat->color, COLOR::LIGHTEST);
    quadTop->SetMaterial(mat);
    quadTop->Update();
    _delete(platform, quadTop);

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
//endregion

