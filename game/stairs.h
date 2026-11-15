#pragma once

#include "../engine/platform/platform.h"
#include "model.h"
#include "util.h"

#include "vector"


void stairsNE(CompositeMaskedGeometryTile* root, int row, int col, int length, int width, int height, int steps);
void stairsNW(CompositeMaskedGeometryTile* root, int row, int col, int length, int width, int height, int steps);
void stairsSW(CompositeMaskedGeometryTile* root, int row, int col, int length, int width, int height, int steps);
void stairsSE(CompositeMaskedGeometryTile* root, int row, int col, int length, int width, int height, int steps);

CompositeMaskedGeometryTile *getStairsMaskedGeometry(
        Platform* platform, Material* mat,
        int row, int col, int length, int width, int height, int steps, Orientation orientation
) {
    auto root = new CompositeMaskedGeometryTile();
    root->row = row;
    root->col = col;
    root->length = length;
    root->width = width;
    root->height = height;
    root->type = STAIRS;
    root->height_base = 0; // todo

    switch(orientation) {
        case NORTHEAST: stairsNE(root, row, col, length, width, height, steps); break;
        case NORTHWEST: stairsNW(root, row, col, length, width, height, steps); break;
        case SOUTHWEST: stairsSW(root, row, col, length, width, height, steps); break;
        case SOUTHEAST: stairsSE(root, row, col, length, width, height, steps); break;
    }

    for (int h = 0; h < root->height; h++) {
        for (int l = 0; l < root->length; l++) {
            for (int w = 0; w < root->width; w++) {

                switch(orientation) {
                    case NORTHEAST: {
                        auto tile = new CompositeMaskedGeometryTileCell();
                        tile->row = row - l;
                        tile->col = col - w;
                        tile->height = h;
                        tile->type = COMPOSITE_MASKED_GEOMETRY_UNIT;
                        tile->parent = root;
                        root->cells.push_back(tile);
                        break;
                    }
                    case NORTHWEST: {
                        auto tile = new CompositeMaskedGeometryTileCell();
                        tile->row = row + w;
                        tile->col = col - l;
                        tile->height = h;
                        tile->type = COMPOSITE_MASKED_GEOMETRY_UNIT;
                        tile->parent = root;
                        root->cells.push_back(tile);
                        break;
                    }
                    case SOUTHWEST: {
                        auto tile = new CompositeMaskedGeometryTileCell();
                        tile->row = row + l;
                        tile->col = col + w;
                        tile->height = h;
                        tile->type = COMPOSITE_MASKED_GEOMETRY_UNIT;
                        tile->parent = root;
                        root->cells.push_back(tile);
                        break;
                    }
                    case SOUTHEAST:
                        auto tile = new CompositeMaskedGeometryTileCell();
                        tile->row = row - w;
                        tile->col = col + l;
                        tile->height = h;
                        tile->type = COMPOSITE_MASKED_GEOMETRY_UNIT;
                        tile->parent = root;
                        root->cells.push_back(tile);
                        break;
                }
            }
        }
    }
    for (auto& face : root->faces) {
        if (face.points.size() == 3) {
            auto tri = getTriangle(platform, face.points[0], face.points[1], face.points[2]);
            SetColor(mat->uniformFields[0].f4, face.color);
            tri->SetMaterial(mat);
            face.tri = tri;
        }
        else if (face.points.size() == 4) {
            auto quad = getQuad(platform, face.points[0], face.points[1], face.points[2], face.points[3]);
            SetColor(mat->uniformFields[0].f4, face.color);
            quad->SetMaterial(mat);
            face.quad = quad;
        }
        else {
            assert(false);
        }
    }
    return root;
}

void stairsNE(
        CompositeMaskedGeometryTile* root,
        int row, int col, int length, int width, int height, int steps
) {
    Point southEast[4];
    getIsoTilePoints(row, col, 0, southEast);
    Point northEast[4];
    getIsoTilePoints(row - (length - 1), col, 0, northEast);
    Point northEastTop[4];
    getIsoTilePoints(row - (length - 1), col, height, northEastTop);
    Point northWestTop[4];
    getIsoTilePoints(row - (length - 1), col - (width - 1), height, northWestTop);
    Point southWest[4];
    getIsoTilePoints(row, col - (width - 1), 0, southWest);

    float xDist = distance(southEast[0], northEast[1]);
    float yDist = distance(northEast[1], northEastTop[1]);
    float stairLength = xDist / (float) steps;
    float stairHeight = yDist / (float) steps;

    Point start = northEastTop[1];
    Point end;
    int sideLength = (int) distance(southEast[0], southEast[3]);
    Point qBottom1 = Point(start.x, start.y + height * TILE_H);
    for (int i = 0; i < steps; i++) {
        // Draw length
        auto vec = multiplyVectorPoint(start, getBasisVector(SOUTHWEST), stairLength);
        end = Point{(int) vec.x, (int) vec.y};
        root->lines.push_back({start, end});

        // Color east face
        auto qBottom2Vec = multiplyVectorPoint(qBottom1, getBasisVector(SOUTHWEST), stairLength);
        auto qBottom2 = Point(qBottom2Vec.x, qBottom2Vec.y);
        root->faces.push_back({{start, end, qBottom2, qBottom1}, GetColor(COLOR::DARK)});

        qBottom1 = Point(qBottom2.x, qBottom2.y);

        // Color top surface
        auto topWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto topWidthPoint = Point{(int) topWidthVec.x, (int) topWidthVec.y};
        auto startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        root->faces.push_back({{start, end, topWidthPoint, startWidthPoint}, GetColor(COLOR::LIGHTEST)});

        // Draw width lines at top of stair
        auto vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        Point pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        root->lines.push_back({end, pointWidthLine});

        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        root->lines.push_back({start, end});

        // Color north face (riser)
        auto northWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto northWidthPoint = Point{(int) northWidthVec.x, (int) northWidthVec.y};
        startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHWEST), (float) (width * sideLength));
        startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        root->faces.push_back({{end, start, startWidthPoint, northWidthPoint}, GetColor(COLOR::MID)});

        start = end;

        // Draw width lines at bottom of stair
        vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        root->lines.push_back({end, pointWidthLine});
    }
    start = northWestTop[2];
    for (int i = 0; i < steps; i++) {
        // Draw length
        auto vec = multiplyVectorPoint(start, getBasisVector(SOUTHWEST), stairLength);
        end = Point{(int) vec.x, (int) vec.y};
        root->lines.push_back({start, end});
        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        root->lines.push_back({start, end});
        start = end;
    }

    root->lines.push_back({southEast[0], northEast[1]});
    root->lines.push_back({northEast[1], northEastTop[1]});
    root->lines.push_back({northEastTop[1], northWestTop[2]});
    root->lines.push_back({southWest[3], southEast[0]});
}

void stairsNW(
        CompositeMaskedGeometryTile* root,
        int row, int col, int length, int width, int height, int steps
) {
    Point southEast[4];
    getIsoTilePoints(row, col, 0, southEast);
    Point northEast[4];
    getIsoTilePoints(row, col - (length - 1), 0, northEast);
    Point northEastTop[4];
    getIsoTilePoints(row, col - (length - 1), height, northEastTop);

    Point southWest[4];
    getIsoTilePoints(row + (width - 1), col, 0, southWest);
    Point northWest[4];
    getIsoTilePoints(row + (width - 1), col - (length - 1), 0, northWest);
    Point northWestTop[4];
    getIsoTilePoints(row + (width - 1), col - (length - 1), height, northWestTop);

    float xDist = distance(southWest[0], northWest[3]);
    float yDist = distance(northWest[3], northWestTop[3]);
    float stairLength = xDist / (float) steps;
    float stairHeight = yDist / (float) steps;

    Point start = northWestTop[3];
    Point end;
    int sideLength = (int) distance(southEast[0], southEast[3]);
    Point qBottom1 = Point(start.x, start.y + height * TILE_H);
    for (int i = 0; i < steps; i++) {
        // Draw length
        auto vec = multiplyVectorPoint(start, getBasisVector(SOUTHEAST), stairLength);
        end = Point{(int) vec.x, (int) vec.y};
        root->lines.push_back({start, end});

        // Color east face
        auto qBottom2Vec = multiplyVectorPoint(qBottom1, getBasisVector(SOUTHEAST), stairLength);
        auto qBottom2 = Point(qBottom2Vec.x, qBottom2Vec.y);
        root->faces.push_back({{start, end, qBottom2, qBottom1}, GetColor(COLOR::DARK)});

        qBottom1 = Point(qBottom2.x, qBottom2.y);

        // Color top surface
        auto topWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHEAST), (float) (width * sideLength));
        auto topWidthPoint = Point{(int) topWidthVec.x, (int) topWidthVec.y};
        auto startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHEAST), (float) (width * sideLength));
        auto startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        root->faces.push_back({{start, end, topWidthPoint, startWidthPoint}, GetColor(COLOR::LIGHTEST)});

        // Draw width lines at top of stair
        auto vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHEAST), (float) (width * sideLength));
        Point pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        root->lines.push_back({end, pointWidthLine});

        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        root->lines.push_back({start, end});

        // Color north face (riser)
        auto northWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHEAST), (float) (width * sideLength));
        auto northWidthPoint = Point{(int) northWidthVec.x, (int) northWidthVec.y};
        startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHEAST), (float) (width * sideLength));
        startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        root->faces.push_back({{end, start, startWidthPoint, northWidthPoint}, GetColor(COLOR::MID)});

        start = end;

        // Draw width lines at bottom of stair
        vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHEAST), (float) (width * sideLength));
        pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        root->lines.push_back({end, pointWidthLine});
    }
    start = northEastTop[2];
    for (int i = 0; i < steps; i++) {
        // Draw length
        auto vec = multiplyVectorPoint(start, getBasisVector(SOUTHEAST), stairLength);
        end = Point{(int) vec.x, (int) vec.y};
        root->lines.push_back({start, end});
        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        root->lines.push_back({start, end});
        start = end;
    }

    root->lines.push_back({southWest[0], northWest[3]});
    root->lines.push_back({northWest[3], northWestTop[3]});
    root->lines.push_back({northWestTop[3], northEastTop[2]});
    root->lines.push_back({southWest[0], southEast[1]});
}

void stairsSW(
        CompositeMaskedGeometryTile* root,
        int row, int col, int length, int width, int height, int steps
) {
    Point southEast[4];
    getIsoTilePoints(row + (length - 1), col + (width - 1), 0, southEast);
    Point southEastTop[4];
    getIsoTilePoints(row + (length - 1), col + (width - 1), height, southEastTop);
    Point northEast[4];
    getIsoTilePoints(row, col + (width - 1), 0, northEast);
    Point northEastTop[4];
    getIsoTilePoints(row, col + (width - 1), height, northEastTop);
    Point southWest[4];
    getIsoTilePoints(row + (length - 1), col + (width - 1), 0, southWest);
    Point southWestTop[4];
    getIsoTilePoints(row + (length - 1), col  + (width - 1), height, southWestTop);
    Point northWest[4];
    getIsoTilePoints(row, col, 0, northWest);
    Point northWestTop[4];
    getIsoTilePoints(row, col, height, northWestTop);

    float xDist = distance(southEast[0], northEast[1]);
    float yDist = distance(southWest[0], southWestTop[0]);
    float stairLength = xDist / (float) steps;
    float stairHeight = yDist / (float) steps;

    Point start = southEastTop[0];
    Point end;
    int sideLength = (int) distance(southEast[0], southEast[3]);
    Point qBottom1 = Point(start.x, start.y + height * TILE_H);

    // Color back face
    auto qBottom3Vec = multiplyVectorPoint(qBottom1, getBasisVector(NORTHWEST), xDist);
    auto qBottom3 = Point(qBottom3Vec.x, qBottom3Vec.y);
    Point qTop3 = Point(qBottom3.x, qBottom3.y - height * TILE_H);
    root->faces.push_back({{start, qBottom1, qBottom3, qTop3}, GetColor(COLOR::MID)});

    for (int i = 0; i < steps; i++) {
        // Draw length
        auto vec = multiplyVectorPoint(start, getBasisVector(NORTHEAST), stairLength);
        end = Point{(int) vec.x, (int) vec.y};
        root->lines.push_back({start, end});

        // Color top surface
        auto topWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto topWidthPoint = Point{(int) topWidthVec.x, (int) topWidthVec.y};
        auto startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        root->lines.push_back({startWidthPoint, topWidthPoint});
        root->faces.push_back({{start, end, topWidthPoint, startWidthPoint}, GetColor(COLOR::LIGHTEST)});

        // Color east face
        auto qBottom2Vec = multiplyVectorPoint(qBottom1, getBasisVector(NORTHEAST), stairLength);
        auto qBottom2 = Point(qBottom2Vec.x, qBottom2Vec.y);
        root->faces.push_back({{start, end, qBottom2, qBottom1}, GetColor(COLOR::DARK)});
        qBottom1 = Point(qBottom2.x, qBottom2.y);

        // Draw width lines at top of stair
        auto vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        Point pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        root->lines.push_back({end, pointWidthLine});

        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        root->lines.push_back({start, end});

        start = end;
    }

    root->lines.push_back({southEast[0], northEast[1]});
    root->lines.push_back({southEast[0], southEastTop[0]});
    root->lines.push_back({southWestTop[3], southEastTop[0]});
    root->lines.push_back({southWest[3], southEast[0]});
}

void stairsSE(
        CompositeMaskedGeometryTile* root,
        int row, int col, int length, int width, int height, int steps
) {
    Point southEast[4];
    getIsoTilePoints(row - (width - 1), col + (length - 1), 0, southEast);
    Point southEastTop[4];
    getIsoTilePoints(row - (width - 1), col + (length - 1), height, southEastTop);
    Point southWest[4];
    getIsoTilePoints(row, col + (length - 1), 0, southWest);
    Point southWestTop[4];
    getIsoTilePoints(row, col + (length - 1), height, southWestTop);

    Point northEast[4];
    getIsoTilePoints(row  - (width - 1), col, 0, northEast);
    Point northWest[4];
    getIsoTilePoints(row, col, 0, northWest);

    float xDist = distance(northWest[3], southWest[0]);
    float yDist = distance(southWest[0], southWestTop[0]);
    int sideLength = (int) distance(northWest[3], northWest[2]);
    float stairLength = xDist / (float) steps;
    float stairHeight = yDist / (float) steps;
    float stairWidth = (width * sideLength);

    Point start = northWest[3];

    for (int i = 0; i < steps; i++) {
        // Draw stair height
        Point stairBottom = Point{start.x, start.y - (i)*(int)stairHeight};
        Point stairHeightTop = Point{stairBottom.x, stairBottom.y - (int)stairHeight};
        root->lines.push_back({stairBottom, stairHeightTop});

        // Draw stair width
        Point stairTopBack = multiplyPointByVector(stairHeightTop, getBasisVector(NORTHEAST), stairWidth);
        root->lines.push_back({stairHeightTop, stairTopBack});

        // Draw stair length front
        Point stairEndFrontTop = multiplyPointByVector(stairHeightTop, getBasisVector(SOUTHEAST), stairLength);
        root->lines.push_back({stairHeightTop, stairEndFrontTop});

        // Draw stair length back
        Point stairEndBack = multiplyPointByVector(stairTopBack, getBasisVector(SOUTHEAST), stairLength);
        root->lines.push_back({stairTopBack, stairEndBack});

        // Draw side face
        Point end = multiplyPointByVector(start, getBasisVector(SOUTHEAST), stairLength);
        root->faces.push_back({{start, stairHeightTop, stairEndFrontTop, end}, GetColor(COLOR::MID)});

        // Draw top face
        root->faces.push_back({{stairHeightTop, stairEndFrontTop, stairEndBack, stairTopBack}, GetColor(COLOR::LIGHTEST)});

        start = end;
    }

    // Color back face
    root->faces.push_back({{southWest[0], southEast[1], southEastTop[1], southWestTop[0]}, GetColor(COLOR::DARK)});

    root->lines.push_back({northWest[3], southWest[0]}); // side length bottom
    root->lines.push_back({southEast[1], southEastTop[1]}); // vertical back east
    root->lines.push_back({southWest[0], southWestTop[0]}); // vertical back west
    root->lines.push_back({southWest[0], southEast[1]}); // back length bottom
    root->lines.push_back({southWestTop[0], southEastTop[1]}); // back length top
}
