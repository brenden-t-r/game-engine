#include "../engine/constants.h"

#include <vector>

const int TILE_W = 64*2;
const int TILE_H = 32*2;
const int GRID_COLS = 10;
const int GRID_ROWS = 10;
const int ORIGIN_X = WINDOW_WIDTH / 2;
const int ORIGIN_Y = 150;
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
        if (id >3) {
            printf(".");
        }
        printf("Create tile %d\n", id);
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
struct CompositeTileFace{
    std::vector<Point>points;
    COLOR color;
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
std::vector<Tile*> tiles{};