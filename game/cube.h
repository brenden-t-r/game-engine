#pragma once

#include "../engine/platform/platform.h"
#include "model.h"
#include "util.h"

#include "vector"

void drawCubeNE(Platform* platform, MaterialColor* mat, CompositePiece* root);

CompositePiece* drawCubeCompositeTile(
        Platform* platform, MaterialColor* mat,
        int row, int col, int length, int width, int height, Orientation orientation
) {
    auto root = new CompositePiece();
    root->row = row;
    root->col = col;
    root->orientation = orientation;
    root->width = width;
    root->length = length;
    root->height = height;
    root->type = COMPOSITE_CUBE;

    switch (orientation) {
        case NORTHEAST: drawCubeNE(platform, mat, root); break;
        case NORTHWEST: break;
        case SOUTHEAST: break;
        case SOUTHWEST: break;
    }

    return root;
}

void drawCubeNE(Platform* platform, MaterialColor* mat, CompositePiece* root) {
    for (int h = 0; h < root->height; h++) {
        for (int l = 0; l < root->length; l++) {
            for (int w = 0; w < root->width; w++) {
                //region setup
                auto *tile = new CompositeTile();
                tile->type = COMPOSITE_CUBE_UNIT;
                tile->row = root->row - l;
                tile->col = root->col - w;
                tile->height = h;
                tile->parent = root;

                Point bottom[4];
                getIsoTilePoints(root->row - l, root->col - w, h, bottom);
                Point top[4];
                getIsoTilePoints(root->row - l, root->col - w, h + 1, top);
                //endregion

                //region draw faces
                // side
                tile->faces.push_back(CompositeTileFace{std::vector<Point> {
                    bottom[0], bottom[1], top[1], top[0]
                }, GetColor(COLOR::DARK)});
                // front
                tile->faces.push_back(CompositeTileFace{std::vector<Point> {
                        bottom[0], top[0], top[3], bottom[3]
                }, GetColor(COLOR::MID)});
                // top
                if (h == root->height-1) {
                    tile->faces.push_back(CompositeTileFace{std::vector<Point>{
                            top[0], top[1], top[2], top[3]
                    }, GetColor(COLOR::LIGHTEST)});
                }
                //endregion

                //region draw lines
                // vert front
                if (w == 0 && l == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[0],  top[0]});
                }
                // vert front
                if (w == root->width - 1 && l == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[3],  top[3]});
                }
                // bottom length
                if (h == 0 && w == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[0], bottom[1]});
                }
                // vert back
                if (l == root->length - 1 && w == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[1], top[1]});
                }
                // bottom width
                if (l == 0 && h == 0) {
                    tile->lines.push_back(CompositeTileLine{bottom[0], bottom[3]});
                }
                // top back
                if (h == root->height - 1 && l == root->length - 1) {
                    tile->lines.push_back(CompositeTileLine{top[1], top[2]});
                }
                // top side
                if (h == root->height - 1 && w == 0) {
                    tile->lines.push_back(CompositeTileLine{top[0], top[1]});
                }
                if (h == root->height - 1 && w == root->width-1) {
                    tile->lines.push_back(CompositeTileLine{top[3], top[2]});
                }
                // top front
                if (h == root->height - 1 && l == 0) {
                    tile->lines.push_back(CompositeTileLine{top[0], top[3]});
                }

                //endregion

                if (!tile->faces.empty() || !tile->lines.empty()) {
                    root->tiles.push_back(tile);
                } else delete tile;
            }
        }
    }
}