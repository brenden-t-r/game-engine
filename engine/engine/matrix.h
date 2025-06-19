#ifndef GAMEPROJECT_MATRIX_H
#define GAMEPROJECT_MATRIX_H

#include "cassert"

#include "vector.h"

struct Matrix3 {
    float _11 = 0, _12 = 0, _13 = 0;
    float _21 = 0, _22 = 0, _23 = 0;
    float _31 = 0, _32 = 0, _33 = 0;

    vec3 Row(int i) {
        switch (i) {
            case 0:
                return vec3{_11, _12, _13};
            case 1:
                return vec3{_21, _22, _23};
            case 2:
                return vec3{_31, _32, _33};
            default:
                assert(false);
        }
    }
    vec3 Col(int i) {
        switch (i) {
            case 0:
                return vec3{_11, _21, _31};
            case 1:
                return vec3{_12, _22, _32};
            case 2:
                return vec3{_13, _23, _33};
            default:
                assert(false);
        }
    }

    Matrix3 multiply(Matrix3 b) {
        return {
                dot(Row(0), b.Col(0)), dot(Row(0), b.Col(1)), dot(Row(0), b.Col(2)),
                dot(Row(1), b.Col(0)), dot(Row(1), b.Col(1)), dot(Row(1), b.Col(2)),
                dot(Row(2), b.Col(0)), dot(Row(2), b.Col(1)), dot(Row(2), b.Col(2))
        };
    }
};

static Matrix3 matrix_multiply(Matrix3 a, Matrix3 b) {
    return a.multiply(b);
}

static vec3 matrix_multiply_vec(Matrix3 mat, vec3 vec) {
    return {
            dot(mat.Row(0), vec), // (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
            dot(mat.Row(1), vec),
            dot(mat.Row(2), vec)
    };
}

// Matrix for converting normalized coordinates (-1 to 1) to screen coordinates (0 to screenWidth/Height)
Matrix3 NormalizedToScreenMatrix = {
        WINDOW_WIDTH * 0.5f,  0,                      WINDOW_WIDTH * 0.5f,
        0,                    -WINDOW_HEIGHT * 0.5f,  WINDOW_HEIGHT * 0.5f,
        0,                    0,                      1
};
// Matrix for screen to normalized coordinates conversion
Matrix3 ScreenToNormalizedMatrix = {
        2.0f / WINDOW_WIDTH,  0,                       -1,
        0,                    -2.0f / WINDOW_HEIGHT,   1,
        0,                    0,                       1
};

static vec3 normalized_to_screen_using_matrix(vec3 vec) {
    return matrix_multiply_vec(NormalizedToScreenMatrix, vec);
}

static vec3 screen_to_normalized_using_matrix(vec3 vec) {
    return matrix_multiply_vec(ScreenToNormalizedMatrix, vec);
}

/*
 * Creates a composite translation, scale and rotation matrix.
 * Accepts position in terms of normalized coordinates.
 *
 *   scale.x * cos  -sin,            position.x
 *   sin            scale.y * cos    position.y
 *   0,             0                1
 */
static Matrix3 matrix_transformation(vec3 position, vec3 scale, vec3 rotation) {
    // Rotation matrix
    float angle = rotation.z * PI/180.0f;
    float cos = cosf(angle);
    float sin = sinf(angle);
    Matrix3 R = {
            cos, -sin, 0,
            sin, cos,  0,
            0,   0,    1,
    };

    // Scale matrix
    Matrix3 S = {
            scale.x, 0,       0,
            0,       scale.y, 0,
            0,       0,       1
    };


    // Translation
    vec3 posScreen = normalized_to_screen(position);
    Matrix3 T = {
            1, 0,  posScreen.x,
            0, 1,  posScreen.y,
            0, 0, 1
    };

    // Scale -> Rotate -> Translate
    Matrix3 CombinedTransformation = T.multiply(R).multiply(S);

    // Adjustment for the origin difference between normalized coordinates and screen coordinates (center vs top-left)
    vec3 screenOrigin = normalized_to_screen({ 0, 0, 0 });
    Matrix3 OriginCenterToTopLeft = {
            1, 0,  -screenOrigin.x,
            0, 1,  -screenOrigin.y,
            0, 0, 1
    };

    // Convert to screen coordinates
    // Translate from origin center to origin top-left
    // Scale -> Rotate -> Translate
    // Convert back to normalized coordinates
    Matrix3 result = ScreenToNormalizedMatrix
        .multiply(CombinedTransformation)
        .multiply(OriginCenterToTopLeft)
        .multiply(NormalizedToScreenMatrix);
#ifdef DEBUG_CLAMP
    result._11 = std::roundf(result._11 * 1e5f) / 1e5f;
    result._12 = std::roundf(result._12 * 1e5f) / 1e5f;
    result._21 = std::roundf(result._21 * 1e5f) / 1e5f;
    result._22 = std::roundf(result._22 * 1e5f) / 1e5f;
#endif
    return result;
}

#endif //GAMEPROJECT_MATRIX_H
