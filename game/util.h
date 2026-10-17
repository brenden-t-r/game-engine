#include "../engine/engine/matrix.h"
#include "../engine/engine/text.h"
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
//void drawIsoGrid(Platform* platform, Material* mat, int height = 0) {
//    for (int row = 0; row < GRID_ROWS; row++) {
//        for (int col = 0; col < GRID_COLS; col++) {
//            int screenX = ORIGIN_X + (col - row) * (TILE_W / 2);
//            int screenY = ORIGIN_Y + (col + row) * (TILE_H / 2) - height * TILE_H;
//            drawIsoTile(platform, mat, screenX, screenY, TILE_W, TILE_H);
//            auto n = screen_to_normalized(vec2{(float)screenX, (float)screenY});
//            RenderText(font, "Hi", n.x, n.y, atlas);
//        }
//    }
//}
vec2 pointToVec2(Point p){
    return screen_to_normalized(vec2{(float)p.x, (float)p.y});
}
Triangle* getTriangle(Platform* platform, vec2 _1, vec2 _2, vec2 _3) {
    auto t = (Triangle*)platform->CreateTriangle();
    t->manualVertices = true;
    t->vertices[0] = {_1.x, _1.y, 0.0};
    t->vertices[1] = {_2.x, _2.y, 0.0};
    t->vertices[2] = {_3.x, _3.y, 0.0};
    return t;
}
Triangle* getTriangle(Platform* platform, Point _1, Point _2, Point _3) {
    return getTriangle(platform, pointToVec2(_1),pointToVec2(_2),pointToVec2(_3));
}
void deleteQuad(Platform* platform, Quad* o) {
    platform->Delete(o->a);
    platform->Delete(o->b);
    delete o;
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
void drawQuadAndCleanUp(Platform* platform, MaterialColor* mat, Quad* quad, Color color) {
    SetColor(mat->color, color);
    quad->SetMaterial(mat);
    quad->Update();
    deleteQuad(platform, quad);
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
void drawCube(Platform* platform, MaterialColor* mat, int row, int col, int height) {
    Point bottom[4];
    getIsoTilePoints(row, col, height, bottom);
    Point top[4];
    getIsoTilePoints(row, col, height+1, top);

    auto quadFront = getQuad(platform, bottom[0], top[0], top[3], bottom[3]);
    SetColor(mat->color, COLOR::MID);
    quadFront->SetMaterial(mat);
    quadFront->Update();
    deleteQuad(platform, quadFront);

    auto quadSide = getQuad(platform, bottom[0], bottom[1], top[1], top[0]);
    SetColor(mat->color, COLOR::DARK);
    quadSide->SetMaterial(mat);
    quadSide->Update();
    deleteQuad(platform, quadSide);

    auto quadTop = getQuad(platform, top[0], top[1], top[2], top[3]);
    SetColor(mat->color, COLOR::LIGHTEST);
    quadTop->SetMaterial(mat);
    quadTop->Update();
    deleteQuad(platform, quadTop);

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

//region: text
Shader* loadFontShader(Platform* platform) {
    ShaderDef shaderDef{};
#if BACKEND_DIRECTX
    shaderDef.path = "assets/shaders/font.hlsl";
    shaderDef.inputLayoutType = InputLayoutType::POSITION_TEXCOORD;
#elif BACKEND_OPENGL
    shaderDef.vertexPath = "assets/shaders/font.glsl.vert";
        shaderDef.fragmentPath = "assets/shaders/font.glsl.frag";
#elif BACKEND_METAL
        shaderDef.vertexPath = "assets/shaders/font.vert.metal";
        shaderDef.fragmentPath = "assets/shaders/font.frag.metal";
#endif
    return platform->LoadShader(shaderDef);
}
Sprite* loadFont(Platform* platform, const char* name, TEXT_MSDF::FontAtlas& _atlas, Shader* shader, TextureSettings textureSettings) {
    std::string png = "assets/sprites/fonts/" + std::string(name) + ".png";
    std::string json = "assets/sprites/fonts/" + std::string(name) + ".json";
    return ::LoadFont(platform, png.c_str(), json.c_str(), _atlas, shader, textureSettings);
}
void setFontSize(Sprite* font, float size) {
    font->SetScale({size/50, size/60, 1.0});
}
void drawText(Sprite* font, TEXT_MSDF::FontAtlas atlas, const char* text, Point p) {
    auto n = screen_to_normalized(vec2{(float)p.x,(float)p.y});
    RenderText(font, text, n.x, n.y, atlas);
}
const char* orientationToStr(Orientation o) {
    switch(o) {
        case NORTHEAST: return "NE";
        case SOUTHEAST: return "SE";
        case SOUTHWEST: return "SW";
        case NORTHWEST: return "NW";
    }
}
const char* selectedPieceStr() {
    switch(selected) {
        case PIECE_TYPE::CUBE: return "Cube";
        case PIECE_TYPE::COMPOSITE_RAMP: return "Ramp Composite";
        default: return "?";
    }
}
//endregion

std::vector<CompositeTile*> drawRampCompositeTile(
        Platform* platform, MaterialColor* mat,
        int row, int col, int length, int width, int height, Orientation orientation
) {
    std::vector<CompositeTile*> cTiles{};
    auto root = new CompositePiece();
    root->row = row;
    root->col = col;
    root->orientation = orientation;
    root->type = COMPOSITE_RAMP_UNIT;
    printf("Creating root %d %d\n", row, col);

    if (orientation == SOUTHEAST) {
        root->col = col+(length-1);

        Point p1[4];
        getIsoTilePoints(row, col, 0, p1);
        Point p2[4];
        getIsoTilePoints(row, col+(length-1), height, p2);
        Point p1a = p1[3];
        Point p2a = p2[0];
//        drawPoint(p1a, 10);
//        drawLineWithEndpoints(p1a, p2a, 5);

        for (int h = 0; h < height; h++) {
            for (int l = 0; l < length; l++) {
                for (int w = 0; w < width; w++) {
                    //region: setup
                    Point bottom[4];
                    getIsoTilePoints(row - w, col + l, h, bottom);
                    Point top[4];
                    getIsoTilePoints(row - w, col + l, h + 1, top);
                    Point sideFace[4];
                    sideFace[0] = bottom[0];
                    sideFace[1] = top[0];
                    sideFace[2] = top[3];
                    sideFace[3] = bottom[3];

                    auto *tile = new CompositeTile();
                    tile->type = COMPOSITE_RAMP_UNIT;
                    tile->row = row - w;
                    tile->col = col + l;
                    tile->height = h;
                    tile->parent = root;
                    //endregion

                    if (true) {
                        //region: draw faces
                        Point p1[4];
                        getIsoTilePoints(row - w, col, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row - w, col + (length -1), height, p2);
                        Point p2a = p1[3];
                        Point p2b = p2[0];
                        Point quad[4] = {
                                bottom[0],
                                top[0],
                                top[3],
                                bottom[3]
                        };

                        //region: top/front face
                        // don't draw if obscured
                        if (height < length) {
                            Point start, end;
                            Point ef2 = sideFace[0];
                            Point ef3 = sideFace[3];
                            bool intersects = false;
                            if (LineQuadIntersection(p2a, p2b, quad, start, end)) {
                                ef2 = start;
                                ef3 = end;
                                intersects = true;
                            }
                            if (intersects) {
                                Point ef2b{ef2.x, ef2.y};
                                Point ef3b{ef3.x, ef3.y};
                                ef2b.x += TILE_W / 2;
                                ef2b.y -= TILE_H / 2;
                                ef3b.x += TILE_W / 2;
                                ef3b.y -= TILE_H / 2;
                                std::vector<Point> facePoints{};
                                addUnique(facePoints, ef2);
                                addUnique(facePoints, ef2b);
                                addUnique(facePoints, ef3);
                                addUnique(facePoints, ef3b);
                                sortPointsAroundCenter(facePoints);
                                tile->faces.push_back(CompositeTileFace{facePoints,  GetColor(COLOR::LIGHTEST)});
                            }
                        }
                        //endregion

                        // side face
                        if (w == 0) {
                            std::vector<Point> points{};
                            if (!isAboveLine(p2a, p2b, sideFace[0])) {
                                addUnique(points, sideFace[0]);
                            }
                            if (!isAboveLine(p2a, p2b, sideFace[1])) {
                                addUnique(points, sideFace[1]);
                            }
                            if (!isAboveLine(p2a, p2b, sideFace[2])) {
                                addUnique(points, sideFace[2]);
                            }
                            if (!isAboveLine(p2a, p2b, sideFace[3])) {
                                addUnique(points, sideFace[3]);
                            }
                            if (!isAboveLine(p2a, p2b, top[3])) {
                                addUnique(points, top[3]);
                            }
                            if (!isAboveLine(p2a, p2b, top[0])) {
                                addUnique(points, top[0]);
                            }
                            Point intersection;
                            if (lineSegmentIntersection(p2a, p2b, bottom[0], bottom[3], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            if (lineSegmentIntersection(p2a, p2b, bottom[0], top[0], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            if (lineSegmentIntersection(p2a, p2b, top[0], top[3], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            if (lineSegmentIntersection(p2a, p2b, bottom[3], top[3], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            sortPointsAroundCenter(points);
                            if (points.size() >= 3) {
                                tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::MID2)});
                            }
                        }

                        // back face
                        if (l == length-1) {
                            std::vector<Point> points{};
                            points = {bottom[0],top[0],top[1],bottom[1]};
                            tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::DARK)});
                        }

                        //endregion
                    }

                    if (true) {
                        //region: draw lines
                        if (w == 0) {
                            Point p1[4];
                            getIsoTilePoints(row - w, col, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row - w, col  + (length - 1), height, p2);
                            Point quad[4] = {
                                    bottom[0],
                                    top[0],
                                    top[3],
                                    bottom[3]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[3], p2[0], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        // don't draw if obscured
                        if (height < length) {
                            if (w == width - 1) {
                                Point p1[4];
                                getIsoTilePoints(row - w, col, 0, p1);
                                Point p2[4];
                                getIsoTilePoints(row - w, col + (length - 1), height, p2);
                                Point quad[4] = {
                                        bottom[1],
                                        top[1],
                                        top[2],
                                        bottom[2]
                                };
                                Point start, end;
                                if (LineQuadIntersection(p1[2], p2[1], quad, start, end)) {
                                    tile->lines.push_back(CompositeTileLine{start, end});
                                }
                            }
                        }
                        if (h == 0 && w == 0) {
                            // bottom side line
                            tile->lines.push_back(CompositeTileLine{bottom[3], bottom[0]});
                        }
                        if (l == length - 1 && w == 0) {
                            // back vert line
                            tile->lines.push_back(CompositeTileLine{bottom[0], top[0]});
                        }
                        if (l == length - 1 && w == width - 1) {
                            // back vert line far
                            tile->lines.push_back(CompositeTileLine{bottom[1], top[1]});
                        }
                        // don't draw if obscured
                        if (height < length) {
                            if (l == 0 && h == 0) {
                                // front bottom line
                                tile->lines.push_back(CompositeTileLine{bottom[3], bottom[2]});
                            }
                        }
                        if (h == 0 && l == length - 1) {
                            // back bottom line
                            tile->lines.push_back(CompositeTileLine{bottom[0], bottom[1]});
                        }
                        if (h == height - 1 && l == length - 1) {
                            // back top line
                            tile->lines.push_back(CompositeTileLine{top[0], top[1]});
                        }
                        //endregion
                    }

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    }
                }
            }
        }
    }

    /*
    if (orientation == SOUTHWEST) {
        root->row = row+(length-1);

        Point p1[4];
        getIsoTilePoints(row, col, 0, p1);
        Point p2[4];
        getIsoTilePoints(row+(length-1), col, height, p2);
        Point p1a = p1[1];
        Point p2a = p2[0];
//        drawPoint(p1a, 10);
//        drawLineWithEndpoints(p1[0], p1[1], 5);
//        drawLineWithEndpoints(p1a, p2a, 5);

        for (int h = 0; h < height; h++) {
            for (int l = 0; l < length; l++) {
                for (int w = 0; w < width; w++) {
                    //region: setup
                    Point bottom[4];
                    getIsoTilePoints(row + l, col - w, h, bottom);
                    Point top[4];
                    getIsoTilePoints(row + l, col - w, h + 1, top);
                    Point sideFace[4];
                    sideFace[0] = bottom[0];
                    sideFace[1] = bottom[1];
                    sideFace[2] = top[1];
                    sideFace[3] = top[0];
//                    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 80);
//                    fillQuad(sideFace[0],sideFace[1],sideFace[2],sideFace[3]);

                    auto *tile = new CompositeTile();
                    tile->type = COMPOSITE_RAMP_UNIT;
                    tile->orientation = orientation;
                    tile->row = row + l;
                    tile->col = col - w;
                    tile->height = h;
                    tile->parent = root;
                    //endregion

                    if (true) {
                        //region: draw faces
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        std::vector<Point> points{};

                        // side face
                        if (w == 0) {
                            setRenderColor(COLOR_DARK);
                            if (isAboveLine(p1a, p2a, sideFace[0])) {
                                addUnique(points, sideFace[0]);
                            }
                            if (isAboveLine(p1a, p2a, sideFace[1])) {
                                addUnique(points, sideFace[1]);
                            }
                            if (isAboveLine(p1a, p2a, sideFace[2])) {
                                addUnique(points, sideFace[2]);
                            }
                            if (isAboveLine(p1a, p2a, sideFace[3])) {
                                addUnique(points, sideFace[3]);
                            }
                            if (isAboveLine(p1a, p2a, top[1])) {
                                addUnique(points, top[1]);
                            }
                            if (isAboveLine(p1a, p2a, top[0])) {
                                addUnique(points, top[0]);
                            }
                            Point intersection;
                            if (lineSegmentIntersection(p1a, p2a, bottom[0], bottom[1], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            if (lineSegmentIntersection(p1a, p2a, bottom[0], top[0], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            if (lineSegmentIntersection(p1a, p2a, top[0], top[1], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            if (lineSegmentIntersection(p1a, p2a, bottom[1], top[1], intersection)) {
                                addUnique(points, Point{intersection.x, intersection.y});
                            }
                            sortPointsAroundCenter(points);
                            if (points.size() >= 3) {
                                tile->faces.push_back(CompositeTileFace{points, COLOR_DARK});
                            }
                        }

                        // back face
                        setRenderColor(COLOR_MID);
                        if (l == length-1) {
                            std::vector<Point> points{};
                            points = {bottom[0],top[0],top[3],bottom[3]};
                            tile->faces.push_back(CompositeTileFace{points, COLOR_MID});
                        }

                        // top/front face
                        setRenderColor(COLOR_LIGHTEST);
                        Point p1[4];
                        getIsoTilePoints(row, col - w, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row + (length -1), col - w, height, p2);
                        Point p2a = p1[1];
                        Point p2b = p2[0];
                        Point quad[4] = {
                                bottom[0],
                                top[0],
                                top[1],
                                bottom[1]
                        };
                        Point start, end;
                        Point ef2 = sideFace[0];
                        Point ef3 = sideFace[3];
                        bool intersects = false;
                        if (LineQuadIntersection(p2a, p2b, quad, start, end)) {
                            ef2 = start;
                            ef3 = end;
                            intersects = true;
                        }
                        if (intersects) {
                            Point ef2b{ef2.x, ef2.y};
                            Point ef3b{ef3.x, ef3.y};
                            ef2b.x -= TILE_W / 2;
                            ef2b.y -= TILE_H / 2;
                            ef3b.x -= TILE_W / 2;
                            ef3b.y -= TILE_H / 2;
                            std::vector<Point> facePoints{};
                            addUnique(facePoints, ef2);
                            addUnique(facePoints, ef2b);
                            addUnique(facePoints, ef3);
                            addUnique(facePoints, ef3b);
                            sortPointsAroundCenter(facePoints);
                            tile->faces.push_back(CompositeTileFace{facePoints, COLOR_LIGHTEST});
                        }
                        //endregion
                    }

                    if (true) {
                        //region: draw lines
                        if (w == 0) {
                            Point p1[4];
                            getIsoTilePoints(row, col, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row + (length - 1), col, height, p2);
                            Point quad[4] = {
                                    bottom[0],
                                    top[0],
                                    top[1],
                                    bottom[1]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[1], p2[0], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        if (w == width - 1) {
                            Point p1[4];
                            getIsoTilePoints(row, col - width, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row + (length - 1), col - width, height, p2);
                            Point quad[4] = {
                                    bottom[2],
                                    top[2],
                                    top[3],
                                    bottom[3]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[1], p2[0], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        if (h == 0 && w == 0) {
                            // bottom side line
                            tile->lines.push_back(CompositeTileLine{bottom[1], bottom[0]});
                        }
                        if (l == length - 1 && w == 0) {
                            // back vert line
                            tile->lines.push_back(CompositeTileLine{bottom[0], top[0]});
                        }
                        if (l == length - 1 && w == width - 1) {
                            // back vert line far
                            tile->lines.push_back(CompositeTileLine{bottom[3], top[3]});
                        }
                        if (l == 0 && h == 0) {
                            // front bottom line
                            tile->lines.push_back(CompositeTileLine{bottom[1], bottom[2]});
                        }
                        if (h == 0 && l == length - 1) {
                            // back bottom line
                            tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                        }
                        if (h == height - 1 && l == length - 1) {
                            // back top line
                            tile->lines.push_back(CompositeTileLine{top[0], top[3]});
                        }
                        //endregion
                    }

//                    for (auto face : tile->faces) {
//                        setRenderColor(face.color);
//                        auto points = face.points;
//                        if (points.size() == 3) {
//                            fillTriangle(points[0],points[1],points[2], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                        } else if (points.size() == 4) {
//                            fillQuad(points[0],points[1],points[2],points[3], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                        }
//                        else if (points.size() == 5) {
//                            fillQuad(points[0],points[1],points[2],points[3], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                            fillQuad(points[1],points[2],points[3],points[4], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                            fillQuad(points[0],points[1],points[3],points[4], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                            fillQuad(points[0],points[1],points[2],points[4], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                            fillQuad(points[0],points[2],points[3],points[4], SDL_FColor(face.color.r, face.color.g, face.color.b, face.color.a));
//                        }
//                    }
//                    setRenderColor(COLOR_BLACK);
//                    for (auto line : tile->lines) {
//                        drawLineWithEndpoints(line.a, line.b, 1);
//                    }

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    }
                }
            }
        }
    }

    if (orientation == NORTHWEST) {
        Point p1[4];
        getIsoTilePoints(row, col, 0, p1);
        Point p2[4];
        getIsoTilePoints(row, col - (length - 1), height, p2);
        Point p1a = p1[0];
        Point p2a = p2[3];

        for (int h = 0; h < height; h++) {
            for (int l = 0; l < length; l++) {
                for (int w = 0; w < width; w++) {
                    //region: setup
                    Point bottom[4];
                    getIsoTilePoints(row - w, col - l, h, bottom);
                    Point top[4];
                    getIsoTilePoints(row - w, col - l, h + 1, top);
                    Point westFace[4];
                    westFace[0] = bottom[0];
                    westFace[1] = bottom[3];
                    westFace[2] = top[3];
                    westFace[3] = top[0];
//                    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 80);
//                    fillQuad(westFace[0],westFace[1],westFace[2],westFace[3]);

                    auto *tile = new CompositeTile();
                    tile->type = COMPOSITE_RAMP_UNIT;
                    tile->row = row - w;
                    tile->col = col - l;
                    tile->height = h;
                    tile->parent = root;
                    //endregion

                    if (true) {
                        //region: draw faces
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        std::vector<Point> points{};

                        // side face
                        setRenderColor(COLOR_LIGHTEST);
                        if (isAboveLine(p1a, p2a, westFace[0])) {
                            addUnique(points, westFace[0]);
                        }
                        if (isAboveLine(p1a, p2a, westFace[1])) {
                            addUnique(points, westFace[1]);
                        }
                        if (isAboveLine(p1a, p2a, westFace[2])) {
                            addUnique(points, westFace[2]);
                        }
                        if (isAboveLine(p1a, p2a, westFace[3])) {
                            addUnique(points, westFace[3]);
                        }
                        if (isAboveLine(p1a, p2a, top[1])) {
                            addUnique(points, top[1]);
                        }
                        if (isAboveLine(p1a, p2a, top[0])) {
                            addUnique(points, top[0]);
                        }
                        Point intersection;
                        if (lineSegmentIntersection(p1a, p2a, bottom[0], bottom[3], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        if (lineSegmentIntersection(p1a, p2a, bottom[0], top[0], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        if (lineSegmentIntersection(p1a, p2a, top[0], top[3], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        if (lineSegmentIntersection(p1a, p2a, bottom[3], top[3], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        sortPointsAroundCenter(points);
                        if (points.size() >= 3) {
                            tile->faces.push_back(CompositeTileFace{points, COLOR_MID2});
                        }
//                        if (points.size() == 3) {
//                            fillTriangle(points[0],points[1],points[2]);
//                        } else if (points.size() == 4) {
//                            fillQuad(points[0],points[1],points[2],points[3]);
//                        }
//                        else if (points.size() == 5) {
//                            fillQuad(points[0],points[1],points[2],points[3]);
//                            fillQuad(points[1],points[2],points[3],points[4]);
//                            fillQuad(points[0],points[1],points[3],points[4]);
//                            fillQuad(points[0],points[1],points[2],points[4]);
//                            fillQuad(points[0],points[2],points[3],points[4]);
//                        }
//                        else {
//                            printf("count %zu\n", points.size());
//                        }

                        // front face
                        setRenderColor(COLOR_MID2);
                        Point p1[4];
                        getIsoTilePoints(row -w, col, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row -w, col - (length -1), height, p2);
                        Point p2a = p1[0];
                        Point p2b = p2[3];
                        Point quad[4] = {
                                bottom[0],
                                top[0],
                                top[3],
                                bottom[3]
                        };
                        Point start, end;
                        Point ef2 = westFace[0];
                        Point ef3 = westFace[2];
                        bool intersects = false;
                        if (LineQuadIntersection(p2a, p2b, quad, start, end)) {
                            ef2 = start;
                            ef3 = end;
                            intersects = true;
                        }
                        if (intersects) {

                            Point ef2b{ef2.x, ef2.y};
                            Point ef3b{ef3.x, ef3.y};
                            ef2b.x += TILE_W / 2;
                            ef2b.y -= TILE_H / 2;
                            ef3b.x += TILE_W / 2;
                            ef3b.y -= TILE_H / 2;
                            std::vector<Point> facePoints{};
                            addUnique(facePoints, ef2);
                            addUnique(facePoints, ef2b);
                            addUnique(facePoints, ef3);
                            addUnique(facePoints, ef3b);
                            sortPointsAroundCenter(facePoints);
//                            fillQuad(facePoints[0], facePoints[1], facePoints[2], facePoints[3]);
                            tile->faces.push_back(CompositeTileFace{facePoints, COLOR_LIGHTEST});
                        }
                        //endregion
                    }

                    if (true) {
                        //region: draw lines
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        if (w == 0) {
                            Point p1[4];
                            getIsoTilePoints(row, col, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row, col - (length - 1), height, p2);
                            Point quad[4] = {
                                    bottom[0],
                                    top[0],
                                    top[3],
                                    bottom[3]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[0], p2[3], quad, start, end)) {
//                                drawLineWithEndpoints(start, end, 1);
                                tile->lines.push_back(CompositeTileLine{start, end});
//                            westFace[2] = start;
//                            westFace[3] = end;
                            }
                        }
                        if (w == width - 1) {
                            Point p1[4];
                            getIsoTilePoints(row - width, col, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row - width, col - (length - 1), height, p2);
                            Point quad[4] = {
                                    bottom[1],
                                    top[1],
                                    top[2],
                                    bottom[2]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[0], p2[3], quad, start, end)) {
//                                drawLineWithEndpoints(start, end, 1);
                                tile->lines.push_back(CompositeTileLine{start, end});
//                        eastFace[3] = start;
//                        eastFace[2] = end;
                            }
                        }
                        if (h == 0 && w == 0) {
                            // bottom side line
//                            drawLineWithEndpoints(bottom[0], bottom[3], 1);
                            tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                        }
                        if (l == length - 1 && w == 0) {
                            // back vert line
//                            drawLineWithEndpoints(bottom[3], top[3], 1);
                            tile->lines.push_back(CompositeTileLine{bottom[3], top[3]});
                        }
                        if (l == 0 && h == 0) {
                            // front bottom line
//                            drawLineWithEndpoints(bottom[0], bottom[1], 1);
                            tile->lines.push_back(CompositeTileLine{bottom[0], bottom[1]});
                        }
                        if (h == height - 1 && l == length - 1) {
                            // front top line
//                            drawLineWithEndpoints(top[2], top[3], 1);
                            tile->lines.push_back(CompositeTileLine{top[2], top[3]});
                        }
                        //endregion
                    }

//                    for (auto face : tile->faces) {
//                        setRenderColor(face.color);
//                        auto points = face.points;
//                        if (points.size() == 3) {
//                            fillTriangle(points[0],points[1],points[2]);
//                        } else if (points.size() == 4) {
////                            SDL_SetRenderDrawColor(renderer, h == 0 ? 180 : h == 1 ? 90 : h == 2 ? 30: 0, 0, 0,
////                                                   l == 0 ? 255 : l == 1 ? 190 : l == 2 ? 140: 100
////                                                   );
////                            drawPoint(points[0], 5);
////                            drawPoint(points[1], 5);
////                            drawPoint(points[2], 5);
////                            drawPoint(points[3], 5);
//                            fillQuad(points[0],points[1],points[2],points[3]);
//                        }
//                        else if (points.size() == 5) {
//                            fillQuad(points[0],points[1],points[2],points[3]);
//                            fillQuad(points[1],points[2],points[3],points[4]);
//                            fillQuad(points[0],points[1],points[3],points[4]);
//                            fillQuad(points[0],points[1],points[2],points[4]);
//                            fillQuad(points[0],points[2],points[3],points[4]);
//                        }
//                    }
//                    setRenderColor(COLOR_BLACK);
//                    for (auto line : tile->lines) {
//                        drawLineWithEndpoints(line.a, line.b, 1);
//                    }

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    }
                }
            }
        }
    }

    if (orientation == NORTHEAST) {
        Point p1[4];
        getIsoTilePoints(row, col, 0, p1);
        Point p2[4];
        getIsoTilePoints(row - (length - 1), col, height, p2);

        for (int h = 0; h < height; h++) {
            for (int l = 0; l < length; l++) {
                for (int w = 0; w < width; w++) {
                    Point bottom[4];
                    getIsoTilePoints(row - l, col - w, h, bottom);
                    Point top[4];
                    getIsoTilePoints(row - l, col - w, h + 1, top);
                    Point eastFace[4];
                    if (w == 0) {
                        eastFace[0] = bottom[0];
                        eastFace[1] = bottom[1];
                        eastFace[2] = top[1];
                        eastFace[3] = top[0];
                    }

                    auto tile = new CompositeTile();
                    tile->type = COMPOSITE_RAMP_UNIT;
                    tile->row = row - l;
                    tile->col = col - w;
                    tile->height = h;
                    tile->parent = root;

                    // draw faces
                    if (w > -1) {
                        std::vector<Point> points{};
                        if (!isAboveLine(p1[0], p2[1], eastFace[0])) {
                            addUnique(points, eastFace[0]);
                        }
                        if (!isAboveLine(p1[0], p2[1], eastFace[1])) {
                            addUnique(points, eastFace[1]);
                        }
                        if (!isAboveLine(p1[0], p2[1], eastFace[2])) {
                            addUnique(points, eastFace[2]);
                        }
                        if (!isAboveLine(p1[0], p2[1], eastFace[3])) {
                            addUnique(points, eastFace[3]);
                        }
                        if (!isAboveLine(p1[0], p2[1], top[1])) {
                            addUnique(points, top[1]);
                        }
                        if (!isAboveLine(p1[0], p2[1], top[0])) {
                            addUnique(points, top[0]);
                        }
                        Point intersection;
                        if (lineSegmentIntersection(p1[0], p2[1], bottom[0], bottom[1], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        if (lineSegmentIntersection(p1[0], p2[1], bottom[1], top[1], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        if (lineSegmentIntersection(p1[0], p2[1], top[1], top[0], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }
                        if (lineSegmentIntersection(p1[0], p2[1], bottom[0], top[0], intersection)) {
                            addUnique(points, Point{intersection.x, intersection.y});
                        }

                        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 150);

                        sortPointsAroundCenter(points);
//                    for(auto point: points) {
//                        drawPoint(point.x, point.y, 5);
//                    }
                        if (points.size() >= 3) {
                            tile->faces.push_back(CompositeTileFace{points, COLOR_MID2});
                        }

//                    if (points.size() == 3) {
//                        fillTriangle(points[0],points[1],points[2], COLORF_DARK);
//                    } else if (points.size() == 4) {
//                        fillQuad(points[0],points[1],points[2],points[3], COLORF_MID2);
//                    }
//                    else if (points.size() == 5) {
//                        fillQuad(points[0],points[1],points[2],points[3], COLORF_MID);
//                        fillQuad(points[1],points[2],points[3],points[4], COLORF_MID);
//                        fillQuad(points[0],points[1],points[3],points[4], COLORF_MID);
//                        fillQuad(points[0],points[1],points[2],points[4], COLORF_MID);
//                        fillQuad(points[0],points[2],points[3],points[4], COLORF_MID);
//                    }
//                        else {
//                        printf("count %zu\n", points.size());
//                        }

                        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 150);

                        Point p1b[4];
                        getIsoTilePoints(row, col - w, 0, p1b);
                        Point p2b[4];
                        getIsoTilePoints(row - (length - 1), col - w, height, p2b);
                        Point quad[4] = {
                                bottom[0],
                                top[0],
                                top[1],
                                bottom[1]
                        };
                        Point start, end;
                        Point ef2 = eastFace[2];
                        Point ef3 = eastFace[3];
                        if (LineQuadIntersection(p1b[0], p2b[1], quad, start, end)) {
//                        drawLineWithEndpoints(start, end, 1);
                            ef2 = start;
                            ef3 = end;
                        }
                        if (w > 0 || points.size() >= 3) {
                            Point ef2b{ef2.x, ef2.y};
                            Point ef3b{ef3.x, ef3.y};
                            ef2b.x -= TILE_W / 2;
                            ef2b.y -= TILE_H / 2;
                            ef3b.x -= TILE_W / 2;
                            ef3b.y -= TILE_H / 2;
                            std::vector<Point> facePoints{};
                            addUnique(facePoints, ef2);
                            addUnique(facePoints, ef2b);
                            addUnique(facePoints, ef3);
                            addUnique(facePoints, ef3b);
                            sortPointsAroundCenter(facePoints);
                            fillQuad(facePoints[0], facePoints[1], facePoints[2], facePoints[3], COLORF_BLACK);
                            tile->faces.push_back(CompositeTileFace{facePoints, COLOR_LIGHTEST});
                        }

                    }

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                    // draw lines
                    if (w == 0) {
                        Point p1[4];
                        getIsoTilePoints(row, col, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row - (length - 1), col, height, p2);
                        Point quad[4] = {
                                bottom[0],
                                top[0],
                                top[1],
                                bottom[1]
                        };
                        Point start, end;
                        if (LineQuadIntersection(p1[0], p2[1], quad, start, end)) {
                            drawLineWithEndpoints(start, end, 1);
                            tile->lines.push_back(CompositeTileLine{start, end});
                            eastFace[2] = start;
                            eastFace[3] = end;
                        }
                    }
                    if (w == width - 1) {
                        Point p1[4];
                        getIsoTilePoints(row, col - width, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row - (length - 1), col - width, height, p2);
                        Point quad[4] = {
                                bottom[3],
                                top[3],
                                top[2],
                                bottom[2]
                        };
                        Point start, end;
                        if (LineQuadIntersection(p1[0], p2[1], quad, start, end)) {
                            drawLineWithEndpoints(start, end, 1);
                            tile->lines.push_back(CompositeTileLine{start, end});
//                        eastFace[3] = start;
//                        eastFace[2] = end;
                        }
                    }
                    if (h == 0 && w == 0) {
                        drawLineWithEndpoints(bottom[0], bottom[1], 1);
                        tile->lines.push_back(CompositeTileLine{bottom[0], bottom[1]});
                    }
                    if (l == length - 1 && w == 0) {
                        drawLineWithEndpoints(bottom[1], top[1], 1);
                        tile->lines.push_back(CompositeTileLine{bottom[1], top[1]});
                    }
                    if (l == 0 && h == 0) {
                        drawLineWithEndpoints(bottom[0], bottom[3], 1);
                        tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                    }
                    if (h == height - 1 && l == length - 1) {
                        drawLineWithEndpoints(top[1], top[2], 1);
                        tile->lines.push_back(CompositeTileLine{top[1], top[2]});
                    }

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    }
                }
            }
        }
    }
     */

//    for (auto tile : cTiles) {
//        for (auto face: tile->faces) {
//            auto points = face.points;
//            if (points.size() == 3) {
//                auto t = getTriangle(platform, points[0], points[1], points[2]);
//                SetColor(mat->color, face.color);
//                t->SetMaterial(mat);
//                t->Update();
//                platform->Delete(t);
//            } else if (points.size() == 4) {
//                auto q1 = getQuad(platform, points[0], points[1], points[2], points[3]);
//                drawQuadAndCleanUp(platform, mat, q1, face.color);
//            } else if (points.size() == 5) {
//                auto q1 = getQuad(platform, points[0], points[1], points[2], points[3]);
//                auto q2 = getQuad(platform, points[1], points[2], points[3], points[4]);
//                auto q3 = getQuad(platform, points[0], points[1], points[3], points[4]);
//                auto q4 = getQuad(platform, points[0], points[1], points[2], points[4]);
//                auto q5 = getQuad(platform, points[0], points[2], points[3], points[4]);
//                drawQuadAndCleanUp(platform, mat, q1, face.color);
//                drawQuadAndCleanUp(platform, mat, q2, face.color);
//                drawQuadAndCleanUp(platform, mat, q3, face.color);
//                drawQuadAndCleanUp(platform, mat, q4, face.color);
//                drawQuadAndCleanUp(platform, mat, q5, face.color);
//            }
//        }
//        SetColor(mat->color, COLOR::BLACK);
//        for (auto line: tile->lines) {
//            drawLine(platform, mat, line.a, line.b);
//        }
//    }

    return cTiles;
}