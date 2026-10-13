#include "model.h"

#include <cmath>
#include <algorithm>

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
