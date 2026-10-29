#pragma once

#include "../engine/platform/platform.h"

#include "model.h"
#include "util.h"
#include "vector"

void drawRampSE(CompositePiece* root, std::vector<CompositeTile*>& tiles);
void drawRampSW(CompositePiece* root, std::vector<CompositeTile*>& tiles);
void drawRampNW(CompositePiece* root, std::vector<CompositeTile*>& tiles);
void drawRampNE(CompositePiece* root, std::vector<CompositeTile*>& tiles);

std::vector<CompositeTile*> drawRampCompositeTile(
        Platform* platform, MaterialColor* mat,
        int row, int col, int length, int width, int height,
        Orientation orientation
) {
    auto root = new CompositePiece();
    root->row = row;
    root->col = col;
    root->orientation = orientation;
    root->width = width;
    root->length = length;
    root->height = height;
    root->type = COMPOSITE_RAMP_UNIT;

    std::vector<CompositeTile*> tiles{};

    switch (orientation) {
        case SOUTHEAST:
            drawRampSE(root, tiles);
            break;

        case SOUTHWEST:
            drawRampSW(root, tiles);
            break;

        case NORTHWEST:
            drawRampNW(root, tiles);
            break;

        case NORTHEAST:
            drawRampNE(root, tiles);
            break;
    }

    return tiles;
}

void drawRampNE(
        CompositePiece* root,
        std::vector<CompositeTile*>& tiles
) {
    Point p1[4];
    getIsoTilePoints(root->row, root->col, 0, p1);
    Point p2[4];
    getIsoTilePoints(root->row - (root->length - 1), root->col, root->height, p2);

    for (int h = 0; h < root->height; h++) {
        for (int l = 0; l < root->length; l++) {
            for (int w = 0; w < root->width; w++) {
                Point bottom[4];
                getIsoTilePoints(root->row - l, root->col - w, h, bottom);
                Point top[4];
                getIsoTilePoints(root->row - l, root->col - w, h + 1, top);
                Point eastFace[4];
                if (w == 0) {
                    eastFace[0] = bottom[0];
                    eastFace[1] = bottom[1];
                    eastFace[2] = top[1];
                    eastFace[3] = top[0];
                }

                auto tile = new CompositeTile();
                tile->type = COMPOSITE_RAMP_UNIT;
                tile->row = root->row - l;
                tile->col = root->col - w;
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
                        tile->faces.push_back(CompositeTileFace{points, GetColor(COLOR::DARK)});
                    }

                    Point p1b[4];
                    getIsoTilePoints(root->row, root->col - w, 0, p1b);
                    Point p2b[4];
                    getIsoTilePoints(root->row - (root->length - 1), root->col - w, root->height, p2b);
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

                        // lerp between lightest and X,  X is MID
                        int mh = root->height;
                        int max = 5;
                        if (mh > max) {
                            mh = max;
                        }
                        float lerped = lerp(250, 150, ((float)mh) / max);
                        auto color = Color(lerped/255.0, lerped/255.0, lerped/255.0, 1.0);

                        tile->faces.push_back(CompositeTileFace{facePoints, color});
                    }

                }

                // draw lines
                if (w == 0) {
                    Point p1[4];
                    getIsoTilePoints(root->row, root->col, 0, p1);
                    Point p2[4];
                    getIsoTilePoints(root->row - (root->length - 1), root->col, root->height, p2);
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
                if (w == root->width - 1) {
                    Point p1[4];
                    getIsoTilePoints(root->row, root->col - root->width, 0, p1);
                    Point p2[4];
                    getIsoTilePoints(root->row - (root->length - 1), root->col - root->width, root->height, p2);
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
                if (l == root->length - 1 && w == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[1], top[1]});
                }
                if (l == 0 && h == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                }
                if (h == root->height - 1 && l == root->length - 1) {
                    tile->lines.push_back(CompositeTileLine{top[1], top[2]});
                }

                if (!tile->faces.empty() || !tile->lines.empty()) {
                    tiles.push_back(tile);
                } else delete tile;
            }
        }
    }
}

void drawRampNW(
        CompositePiece* root,
        std::vector<CompositeTile*>& tiles
) {
    int row = root->row;
    int col = root->col;
    int length = root->length;
    int width = root->width;
    int height = root->height;

    Point rampStart[4];
    getIsoTilePoints(row + width - 1, col, 0, rampStart);

    Point rampEnd[4];
    getIsoTilePoints(
            row + width - 1,
            col - (length - 1),
            height,
            rampEnd
    );

    Point rampLineStart = rampStart[0];
    Point rampLineEnd = rampEnd[3];

    for (int h = 0; h < height; h++) {
        for (int l = 0; l < length; l++) {
            for (int w = 0; w < width; w++) {
                // Setup
                Point bottom[4];
                getIsoTilePoints(row + w, col - l, h, bottom);

                Point top[4];
                getIsoTilePoints(row + w, col - l, h + 1, top);

                Point westFace[4] = {
                        bottom[0],
                        bottom[3],
                        top[3],
                        top[0]
                };

                auto* tile = new CompositeTile();
                tile->type = COMPOSITE_RAMP_UNIT;
                tile->orientation = NORTHWEST;
                tile->row = row + w;
                tile->col = col - l;
                tile->height = h;
                tile->parent = root;

                // Side face
                std::vector<Point> points{};

                if (isAboveLine(rampLineStart, rampLineEnd, westFace[0])) {
                    addUnique(points, westFace[0]);
                }
                if (isAboveLine(rampLineStart, rampLineEnd, westFace[1])) {
                    addUnique(points, westFace[1]);
                }
                if (isAboveLine(rampLineStart, rampLineEnd, westFace[2])) {
                    addUnique(points, westFace[2]);
                }
                if (isAboveLine(rampLineStart, rampLineEnd, westFace[3])) {
                    addUnique(points, westFace[3]);
                }
                if (isAboveLine(rampLineStart, rampLineEnd, top[1])) {
                    addUnique(points, top[1]);
                }
                if (isAboveLine(rampLineStart, rampLineEnd, top[0])) {
                    addUnique(points, top[0]);
                }

                Point intersection;

                if (lineSegmentIntersection(
                        rampLineStart, rampLineEnd,
                        bottom[0], bottom[3], intersection)) {
                    addUnique(points, Point{intersection.x, intersection.y});
                }
                if (lineSegmentIntersection(
                        rampLineStart, rampLineEnd,
                        bottom[0], top[0], intersection)) {
                    addUnique(points, Point{intersection.x, intersection.y});
                }
                if (lineSegmentIntersection(
                        rampLineStart, rampLineEnd,
                        top[0], top[3], intersection)) {
                    addUnique(points, Point{intersection.x, intersection.y});
                }
                if (lineSegmentIntersection(
                        rampLineStart, rampLineEnd,
                        bottom[3], top[3], intersection)) {
                    addUnique(points, Point{intersection.x, intersection.y});
                }

                sortPointsAroundCenter(points);

                if (points.size() >= 3) {
                    tile->faces.push_back(
                            CompositeTileFace{
                                    points,
                                    GetColor(COLOR::MID)
                            }
                    );
                }

                // Front face
                Point frontP1[4];
                getIsoTilePoints(row + w, col, 0, frontP1);

                Point frontP2[4];
                getIsoTilePoints(
                        row + w,
                        col - (length - 1),
                        height,
                        frontP2
                );

                Point frontLineStart = frontP1[0];
                Point frontLineEnd = frontP2[3];

                Point quad[4] = {
                        bottom[0],
                        top[0],
                        top[3],
                        bottom[3]
                };

                Point start, end;
                Point ef2 = westFace[0];
                Point ef3 = westFace[2];

                if (LineQuadIntersection(
                        frontLineStart, frontLineEnd,
                        quad, start, end)) {
                    ef2 = start;
                    ef3 = end;

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

                    // Shade according to ramp height.
                    int mh = height;
                    int max = 5;

                    if (mh > max) {
                        mh = max;
                    }

                    float lerped = lerp(250, 60.0, ((float)mh) / max);
                    auto color = Color(
                            lerped / 255.0,
                            lerped / 255.0,
                            lerped / 255.0,
                            1.0
                    );

                    tile->faces.push_back(
                            CompositeTileFace{facePoints, color}
                    );
                }

                // Draw lines

                // Near-side ramp boundary
                if (w == 0) {
                    Point p1[4];
                    getIsoTilePoints(row, col, 0, p1);

                    Point p2[4];
                    getIsoTilePoints(
                            row,
                            col - (length - 1),
                            height,
                            p2
                    );

                    Point lineQuad[4] = {
                            bottom[1],
                            top[1],
                            top[2],
                            bottom[2]
                    };

                    Point lineStart, lineEnd;

                    if (LineQuadIntersection(
                            p1[1], p2[2],
                            lineQuad,
                            lineStart, lineEnd)) {
                        tile->lines.push_back(
                                CompositeTileLine{lineStart, lineEnd}
                        );
                    }
                }

                // Far-side ramp boundary
                if (w == width - 1) {
                    Point p1[4];
                    getIsoTilePoints(row + width - 1, col, 0, p1);

                    Point p2[4];
                    getIsoTilePoints(
                            row + width - 1,
                            col - (length - 1),
                            height,
                            p2
                    );

                    Point lineQuad[4] = {
                            bottom[0],
                            top[0],
                            top[3],
                            bottom[3]
                    };

                    Point lineStart, lineEnd;

                    if (LineQuadIntersection(
                            p1[0], p2[3],
                            lineQuad,
                            lineStart, lineEnd)) {
                        tile->lines.push_back(
                                CompositeTileLine{lineStart, lineEnd}
                        );
                    }
                }

                if (h == 0 && w == width - 1) {
                    // Bottom side line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[0], bottom[3]}
                    );
                }

                if (l == length - 1 && w == width - 1) {
                    // Back vertical line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[3], top[3]}
                    );
                }

                if (l == 0 && h == 0) {
                    // Front bottom line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[0], bottom[1]}
                    );
                }

                if (h == height - 1 && l == length - 1) {
                    // Front top line
                    tile->lines.push_back(
                            CompositeTileLine{top[2], top[3]}
                    );
                }

                if (!tile->faces.empty() || !tile->lines.empty()) {
                    tiles.push_back(tile);
                } else {
                    delete tile;
                }
            }
        }
    }
}

void drawRampSW(
        CompositePiece* root,
        std::vector<CompositeTile*>& tiles
) {
    int row = root->row;
    int col = root->col;
    int length = root->length;
    int width = root->width;
    int height = root->height;

    Point rampStart[4];
    getIsoTilePoints(row, col, 0, rampStart);

    Point rampEnd[4];
    getIsoTilePoints(row + (length - 1), col, height, rampEnd);

    for (int h = 0; h < height; h++) {
        for (int l = 0; l < length; l++) {
            for (int w = 0; w < width; w++) {
                // Setup
                Point bottom[4];
                getIsoTilePoints(row + l, col + w, h, bottom);

                Point top[4];
                getIsoTilePoints(row + l, col + w, h + 1, top);

                Point sideFace[4] = {
                        bottom[0],
                        bottom[1],
                        top[1],
                        top[0]
                };

                auto* tile = new CompositeTile();
                tile->type = COMPOSITE_RAMP_UNIT;
                tile->orientation = SOUTHWEST;
                tile->row = row + l;
                tile->col = col + w;
                tile->height = h;
                tile->parent = root;

                // Draw faces
                std::vector<Point> points{};

                // Top/front face
                Point p1[4];
                getIsoTilePoints(row, col + w, 0, p1);

                Point p2[4];
                getIsoTilePoints(row + (length - 1), col + w, height, p2);

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

                    tile->faces.push_back(
                            CompositeTileFace{
                                    facePoints,
                                    GetColor(COLOR::LIGHTEST)
                            }
                    );
                }

                // Side face
                if (w == width - 1) {
                    Point sideP1[4];
                    getIsoTilePoints(row, col + w, 0, sideP1);

                    Point sideP2[4];
                    getIsoTilePoints(
                            row + (length - 1),
                            col + w,
                            height,
                            sideP2
                    );

                    Point sideP1a = sideP1[1];
                    Point sideP2a = sideP2[0];

                    if (isAboveLine(sideP1a, sideP2a, sideFace[0])) {
                        addUnique(points, sideFace[0]);
                    }
                    if (isAboveLine(sideP1a, sideP2a, sideFace[1])) {
                        addUnique(points, sideFace[1]);
                    }
                    if (isAboveLine(sideP1a, sideP2a, sideFace[2])) {
                        addUnique(points, sideFace[2]);
                    }
                    if (isAboveLine(sideP1a, sideP2a, sideFace[3])) {
                        addUnique(points, sideFace[3]);
                    }
                    if (isAboveLine(sideP1a, sideP2a, top[1])) {
                        addUnique(points, top[1]);
                    }
                    if (isAboveLine(sideP1a, sideP2a, top[0])) {
                        addUnique(points, top[0]);
                    }

                    Point intersection;

                    if (lineSegmentIntersection(
                            sideP1a, sideP2a,
                            bottom[0], bottom[1],
                            intersection)) {
                        addUnique(points, Point{intersection.x, intersection.y});
                    }

                    if (lineSegmentIntersection(
                            sideP1a, sideP2a,
                            bottom[0], top[0],
                            intersection)) {
                        addUnique(points, Point{intersection.x, intersection.y});
                    }

                    if (lineSegmentIntersection(
                            sideP1a, sideP2a,
                            top[0], top[1],
                            intersection)) {
                        addUnique(points, Point{intersection.x, intersection.y});
                    }

                    if (lineSegmentIntersection(
                            sideP1a, sideP2a,
                            bottom[1], top[1],
                            intersection)) {
                        addUnique(points, Point{intersection.x, intersection.y});
                    }

                    sortPointsAroundCenter(points);

                    if (points.size() >= 3) {
                        tile->faces.push_back(
                                CompositeTileFace{
                                        points,
                                        GetColor(COLOR::DARK)
                                }
                        );
                    }
                }

                // Back face
                if (l == length - 1) {
                    std::vector<Point> facePoints = {
                            bottom[0],
                            top[0],
                            top[3],
                            bottom[3]
                    };

                    tile->faces.push_back(
                            CompositeTileFace{
                                    facePoints,
                                    GetColor(COLOR::MID)
                            }
                    );
                }

                // Draw lines

                // Ramp boundary along the near side
                if (w == 0) {
                    Point p1[4];
                    getIsoTilePoints(row, col, 0, p1);

                    Point p2[4];
                    getIsoTilePoints(
                            row + (length - 1),
                            col,
                            height,
                            p2
                    );

                    Point lineQuad[4] = {
                            bottom[3],
                            bottom[2],
                            top[2],
                            top[3]
                    };

                    Point lineStart, lineEnd;

                    if (LineQuadIntersection(
                            p1[2], p2[3],
                            lineQuad,
                            lineStart, lineEnd)) {
                        tile->lines.push_back(
                                CompositeTileLine{lineStart, lineEnd}
                        );
                    }
                }

                // Ramp boundary along the far side
                if (w == width - 1) {
                    Point p1[4];
                    getIsoTilePoints(row, col + width - 1, 0, p1);

                    Point p2[4];
                    getIsoTilePoints(
                            row + (length - 1),
                            col + width - 1,
                            height,
                            p2
                    );

                    Point lineQuad[4] = {
                            bottom[0],
                            bottom[1],
                            top[1],
                            top[0]
                    };

                    Point lineStart, lineEnd;

                    if (LineQuadIntersection(
                            p1[1], p2[0],
                            lineQuad,
                            lineStart, lineEnd)) {
                        tile->lines.push_back(
                                CompositeTileLine{lineStart, lineEnd}
                        );
                    }
                }

                if (h == 0 && w == width - 1) {
                    // Bottom side line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[1], bottom[0]}
                    );
                }

                if (l == length - 1 && w == 0) {
                    // Back vertical line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[3], top[3]}
                    );
                }

                if (l == length - 1 && w == width - 1) {
                    // Back vertical line, far side
                    tile->lines.push_back(
                            CompositeTileLine{bottom[0], top[0]}
                    );
                }

                if (l == 0 && h == 0) {
                    // Front bottom line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[1], bottom[2]}
                    );
                }

                if (h == 0 && l == length - 1) {
                    // Back bottom line
                    tile->lines.push_back(
                            CompositeTileLine{bottom[0], bottom[3]}
                    );
                }

                if (h == height - 1 && l == length - 1) {
                    // Back top line
                    tile->lines.push_back(
                            CompositeTileLine{top[0], top[3]}
                    );
                }

                if (!tile->faces.empty() || !tile->lines.empty()) {
                    tiles.push_back(tile);
                } else {
                    delete tile;
                }
            }
        }
    }
}

void drawRampSE(
        CompositePiece* root, std::vector<CompositeTile*>& tiles
) {
    int row = root->row;
    int col = root->col;
    int length = root->length;
    int width = root->width;
    int height = root->height;

    for (int h = 0; h < height; h++) {
        for (int l = 0; l < length; l++) {
            for (int w = 0; w < width; w++) {

                // Setup
                Point bottom[4];
                getIsoTilePoints(row - w, col + l, h, bottom);

                Point top[4];
                getIsoTilePoints(row - w, col + l, h + 1, top);

                Point sideFace[4] = {
                        bottom[0], top[0], top[3], bottom[3]
                };

                auto tile = new CompositeTile();
                tile->type = COMPOSITE_RAMP_UNIT;
                tile->row = row - w;
                tile->col = col + l;
                tile->height = h;
                tile->parent = root;

                // Ramp boundary
                Point p1[4];
                getIsoTilePoints(row - w, col, 0, p1);

                Point p2[4];
                getIsoTilePoints(
                        row - w, col + (length - 1), height, p2
                );

                Point p2a = p1[3];
                Point p2b = p2[0];

                Point quad[4] = {
                        bottom[0], top[0], top[3], bottom[3]
                };

                // Top/front face
                if (height < length) {
                    Point start, end;
                    Point ef2 = sideFace[0];
                    Point ef3 = sideFace[3];

                    bool intersects = false;

                    if (LineQuadIntersection(
                            p2a, p2b, quad, start, end
                    )) {
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

                        tile->faces.push_back(CompositeTileFace{
                                facePoints, GetColor(COLOR::LIGHTEST)
                        });
                    }
                }

                // Side face
                if (w == 0) {
                    std::vector<Point> points{};

                    if (!isAboveLine(p2a, p2b, sideFace[0]))
                        addUnique(points, sideFace[0]);

                    if (!isAboveLine(p2a, p2b, sideFace[1]))
                        addUnique(points, sideFace[1]);

                    if (!isAboveLine(p2a, p2b, sideFace[2]))
                        addUnique(points, sideFace[2]);

                    if (!isAboveLine(p2a, p2b, sideFace[3]))
                        addUnique(points, sideFace[3]);

                    if (!isAboveLine(p2a, p2b, top[3]))
                        addUnique(points, top[3]);

                    if (!isAboveLine(p2a, p2b, top[0]))
                        addUnique(points, top[0]);

                    Point intersection;

                    if (lineSegmentIntersection(
                            p2a, p2b, bottom[0], bottom[3], intersection
                    ))
                        addUnique(points, intersection);

                    if (lineSegmentIntersection(
                            p2a, p2b, bottom[0], top[0], intersection
                    ))
                        addUnique(points, intersection);

                    if (lineSegmentIntersection(
                            p2a, p2b, top[0], top[3], intersection
                    ))
                        addUnique(points, intersection);

                    if (lineSegmentIntersection(
                            p2a, p2b, bottom[3], top[3], intersection
                    ))
                        addUnique(points, intersection);

                    sortPointsAroundCenter(points);

                    if (points.size() >= 3) {
                        tile->faces.push_back(CompositeTileFace{
                                points, GetColor(COLOR::MID2)
                        });
                    }
                }

                // Back face
                if (l == length - 1) {
                    std::vector<Point> points = {
                            bottom[0], top[0], top[1], bottom[1]
                    };

                    tile->faces.push_back(CompositeTileFace{
                            points, GetColor(COLOR::DARK)
                    });
                }

                // Lines
                if (w == 0) {
                    Point quadLine[4] = {
                            bottom[0], top[0], top[3], bottom[3]
                    };

                    Point start, end;

                    if (LineQuadIntersection(
                            p1[3], p2[0], quadLine, start, end
                    )) {
                        tile->lines.push_back(
                                CompositeTileLine{start, end}
                        );
                    }
                }

                // Opposite ramp boundary
                if (height < length && w == width - 1) {
                    Point p1b[4];
                    getIsoTilePoints(row - w, col, 0, p1b);

                    Point p2b[4];
                    getIsoTilePoints(
                            row - w, col + (length - 1), height, p2b
                    );

                    Point quadLine[4] = {
                            bottom[1], top[1], top[2], bottom[2]
                    };

                    Point start, end;

                    if (LineQuadIntersection(
                            p1b[2], p2b[1], quadLine, start, end
                    )) {
                        tile->lines.push_back(
                                CompositeTileLine{start, end}
                        );
                    }
                }

                if (h == 0 && w == 0) {
                    tile->lines.push_back(
                            CompositeTileLine{bottom[3], bottom[0]}
                    );
                }

                if (l == length - 1 && w == 0) {
                    tile->lines.push_back(
                            CompositeTileLine{bottom[0], top[0]}
                    );
                }

                if (l == length - 1 && w == width - 1) {
                    tile->lines.push_back(
                            CompositeTileLine{bottom[1], top[1]}
                    );
                }

                if (height < length && l == 0 && h == 0) {
                    tile->lines.push_back(
                            CompositeTileLine{bottom[3], bottom[2]}
                    );
                }

                if (h == 0 && l == length - 1) {
                    tile->lines.push_back(
                            CompositeTileLine{bottom[0], bottom[1]}
                    );
                }

                if (h == height - 1 && l == length - 1) {
                    tile->lines.push_back(
                            CompositeTileLine{top[0], top[1]}
                    );
                }

                // Keep only tiles that contribute geometry
                if (!tile->faces.empty() || !tile->lines.empty()) {
                    tiles.push_back(tile);
                } else {
                    delete tile;
                }
            }
        }
    }
}
