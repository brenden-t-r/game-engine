#pragma once

#include "../engine/platform/platform.h"
#include "model.h"
#include "util.h"

#include "vector"


void stairsNE(CompositeMaskedGeometryTile* root, int row, int col, int length, int width, int height, int steps);

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

    stairsNE(root, row, col, length, width, height, steps);

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
                    case NORTHWEST:
                        break;
                    case SOUTHWEST:
                        break;
                    case SOUTHEAST:
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


void drawStairs(
        Platform *platform, Material *mat,
        int row, int col, int length, int width, int height, int steps, Orientation orientation) {
    assert(orientation == NORTHEAST);
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
        //drawLine(platform, mat, start, end);

        // Color east face
        auto qBottom2Vec = multiplyVectorPoint(qBottom1, getBasisVector(SOUTHWEST), stairLength);
        auto qBottom2 = Point(qBottom2Vec.x, qBottom2Vec.y);
        SetColor(mat->uniformFields[0].f4, GetColor(COLOR::BLACK));
        auto q = getQuad(platform, start, end, qBottom2, qBottom1);
        q->SetMaterial(mat);
        q->Update();
        delete q;

        qBottom1 = Point(qBottom2.x, qBottom2.y);

        // Color top surface
        auto topWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto topWidthPoint = Point{(int) topWidthVec.x, (int) topWidthVec.y};
        auto startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        SetColor(mat->uniformFields[0].f4, GetColor(COLOR::LIGHTEST));
        q = getQuad(platform, start, end, topWidthPoint, startWidthPoint);
        q->SetMaterial(mat);
        q->Update();
        delete q;

        // Draw width lines at top of stair
        auto vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        Point pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        //drawLine(platform, mat, end, pointWidthLine);

        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        //drawLine(platform, mat, start, end);

        // Color north face (riser)
        auto northWidthVec = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        auto northWidthPoint = Point{(int) northWidthVec.x, (int) northWidthVec.y};
        startWidthVec = multiplyVectorPoint(start, getBasisVector(NORTHWEST), (float) (width * sideLength));
        startWidthPoint = Point{(int) startWidthVec.x, (int) startWidthVec.y};
        SetColor(mat->uniformFields[0].f4, GetColor(COLOR::MID));
        q = getQuad(platform, end, start, startWidthPoint, northWidthPoint);
        q->SetMaterial(mat);
        q->Update();
        delete q;

        start = end;

        // Draw width lines at bottom of stair
        vecWidthLine = multiplyVectorPoint(end, getBasisVector(NORTHWEST), (float) (width * sideLength));
        pointWidthLine = Point{(int) vecWidthLine.x, (int) vecWidthLine.y};
        //drawLine(platform, mat, end, pointWidthLine);
    }
    start = northWestTop[2];
    for (int i = 0; i < steps; i++) {
        // Draw length
        auto vec = multiplyVectorPoint(start, getBasisVector(SOUTHWEST), stairLength);
        end = Point{(int) vec.x, (int) vec.y};
        //drawLine(platform, mat, start, end);
        // Draw height
        start = end;
        end = Point{start.x, start.y + (int) stairHeight};
        //drawLine(platform, mat, start, end);
        start = end;
    }

    //drawLine(platform, mat, southEast[0], northEast[1]);
    //drawLine(platform, mat, northEast[1], northEastTop[1]);
    //drawLine(platform, mat, northEastTop[1], northWestTop[2]);
    //drawLine(platform, mat, southWest[3], southEast[0]);
}