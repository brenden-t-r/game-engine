#pragma once

#include "../engine/platform/platform.h"
#include "model.h"
#include "util.h"

#include "vector"

std::vector<CompositeTile*> drawRampCompositeTile(
        Platform* platform, MaterialColor* mat,
        int row, int col, int length, int width, int height, Orientation orientation
) {
    std::vector<CompositeTile*> cTiles{};
    auto root = new CompositePiece();
    root->row = row;
    root->col = col;
    root->orientation = orientation;
    root->width = width;
    root->length = length;
    root->height = height;
    root->type = COMPOSITE_RAMP_UNIT;

    if (orientation == SOUTHEAST) {
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

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    } else delete tile;
                }
            }
        }
    }

    if (orientation == SOUTHWEST) {

        Point p1[4];
        getIsoTilePoints(row, col, 0, p1);
        Point p2[4];
        getIsoTilePoints(row+(length-1), col, height, p2);
        Point p1a = p1[1];
        Point p2a = p2[0];

        for (int h = 0; h < height; h++) {
            for (int l = 0; l < length; l++) {
                for (int w = 0; w < width; w++) {
                    //region: setup
                    Point bottom[4];
                    getIsoTilePoints(row + l, col + w, h, bottom);
                    Point top[4];
                    getIsoTilePoints(row + l, col + w, h + 1, top);
                    Point sideFace[4];
                    sideFace[0] = bottom[0];
                    sideFace[1] = bottom[1];
                    sideFace[2] = top[1];
                    sideFace[3] = top[0];

                    auto *tile = new CompositeTile();
                    tile->type = COMPOSITE_RAMP_UNIT;
                    tile->orientation = orientation;
                    tile->row = row + l;
                    tile->col = col + w;
                    tile->height = h;
                    tile->parent = root;
                    //endregion

                    if (true) {
                        //region: draw faces
                        std::vector<Point> points{};

                        // top/front face
                        Point p1[4];
                        getIsoTilePoints(row, col + w, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row + (length -1), col + w, height, p2);
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
                            tile->faces.push_back(CompositeTileFace{facePoints, GetColor(COLOR::LIGHTEST)});
                        }

                        // side face
                        if (w == width - 1) {
                            Point p1[4];
                            getIsoTilePoints(row, col + w, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row + (length -1), col + w, height, p2);
                            Point p1a = p1[1];
                            Point p2a = p2[0];
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
                                tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::DARK)});
                            }
                        }

                        // back face
                        if (l == length-1) {
                            std::vector<Point> points{};
                            points = {bottom[0],top[0],top[3],bottom[3]};
                            tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::MID)});
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
                                    bottom[3],
                                    bottom[2],
                                    top[2],
                                    top[3]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[2], p2[3], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        if (w == width - 1) {
                            Point p1[4];
                            getIsoTilePoints(row, col + width - 1, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row + (length - 1), col + width - 1, height, p2);
                            Point quad[4] = {
                                    bottom[0],
                                    bottom[1],
                                    top[1],
                                    top[0]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[1], p2[0], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        if (h == 0 && w == width - 1) {
                            // bottom side line
                            tile->lines.push_back(CompositeTileLine{bottom[1], bottom[0]});
                        }
                        if (l == length - 1 && w == 0) {
                            // back vert line
                            tile->lines.push_back(CompositeTileLine{bottom[3], top[3]});
                        }
                        if (l == length - 1 && w == width - 1) {
                            // back vert line far
                            tile->lines.push_back(CompositeTileLine{bottom[0], top[0]});
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

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    } else delete tile;
                }
            }
        }
    }

    if (orientation == NORTHWEST) {
        Point p1[4];
        getIsoTilePoints(row + width - 1, col, 0, p1);
        Point p2[4];
        getIsoTilePoints(row + width - 1, col - (length - 1), height, p2);
        Point p1a = p1[0];
        Point p2a = p2[3];

        for (int h = 0; h < height; h++) {
            for (int l = 0; l < length; l++) {
                for (int w = 0; w < width; w++) {
                    //region: setup
                    Point bottom[4];
                    getIsoTilePoints(row + w, col - l, h, bottom);
                    Point top[4];
                    getIsoTilePoints(row + w, col - l, h + 1, top);
                    Point westFace[4];
                    westFace[0] = bottom[0];
                    westFace[1] = bottom[3];
                    westFace[2] = top[3];
                    westFace[3] = top[0];

                    auto *tile = new CompositeTile();
                    tile->type = COMPOSITE_RAMP_UNIT;
                    tile->row = row + w;
                    tile->col = col - l;
                    tile->height = h;
                    tile->parent = root;
                    //endregion

                    if (true) {
                        //region: draw faces
                        std::vector<Point> points{};

                        // side face
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
                            tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::MID)});
                        }

                        // front face
                        Point p1[4];
                        getIsoTilePoints(row + w, col, 0, p1);
                        Point p2[4];
                        getIsoTilePoints(row + w, col - (length -1), height, p2);
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
                            tile->faces.push_back(CompositeTileFace{facePoints, GetColor(COLOR::LIGHTEST)});
                        }
                        //endregion
                    }

                    if (true) {
                        //region: draw lines
                        if (w == 0) {
                            Point p1[4];
                            getIsoTilePoints(row, col, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row, col - (length - 1), height, p2);
                            Point quad[4] = {
                                    bottom[1],
                                    top[1],
                                    top[2],
                                    bottom[2]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[1], p2[2], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        if (w == width - 1) {
                            Point p1[4];
                            getIsoTilePoints(row + width - 1, col, 0, p1);
                            Point p2[4];
                            getIsoTilePoints(row + width - 1, col - (length - 1), height, p2);
                            Point quad[4] = {
                                    bottom[0],
                                    top[0],
                                    top[3],
                                    bottom[3]
                            };
                            Point start, end;
                            if (LineQuadIntersection(p1[0], p2[3], quad, start, end)) {
                                tile->lines.push_back(CompositeTileLine{start, end});
                            }
                        }
                        if (h == 0 && w == width - 1) {
                            // bottom side line
                            tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                        }
                        if (l == length - 1 && w == width - 1) {
                            // back vert line
                            tile->lines.push_back(CompositeTileLine{bottom[3], top[3]});
                        }
                        if (l == 0 && h == 0) {
                            // front bottom line
                            tile->lines.push_back(CompositeTileLine{bottom[0], bottom[1]});
                        }
                        if (h == height - 1 && l == length - 1) {
                            // front top line
                            tile->lines.push_back(CompositeTileLine{top[2], top[3]});
                        }
                        //endregion
                    }

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    } else delete tile;
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
                        sortPointsAroundCenter(points);
                        if (points.size() >= 3) {
                            tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::BLACK)});
                        }

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
                            tile->faces.push_back(CompositeTileFace{facePoints, GetColor(COLOR::LIGHTEST)});
                        }

                    }

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
                            tile->lines.push_back(CompositeTileLine{start, end});
                        }
                    }
                    if (h == 0 && w == 0) {
                        tile->lines.push_back(CompositeTileLine{bottom[0], bottom[1]});
                    }
                    if (l == length - 1 && w == 0) {
                        tile->lines.push_back(CompositeTileLine{bottom[1], top[1]});
                    }
                    if (l == 0 && h == 0) {
                        tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                    }
                    if (h == height - 1 && l == length - 1) {
                        tile->lines.push_back(CompositeTileLine{top[1], top[2]});
                    }

                    if (!tile->faces.empty() || !tile->lines.empty()) {
                        cTiles.push_back(tile);
                    } else delete tile;
                }
            }
        }
    }

    return cTiles;
}

