#ifndef GAMEENGINE_VECTOR_H
#define GAMEENGINE_VECTOR_H

#include "cmath"
#include "../constants.h"

constexpr float PI = 3.14159265358979323846f;
constexpr float DEGREES_TO_RADIANS = PI/180;

struct vec3 {
    float x;
    float y;
    float z;
};
struct vec2 {
    float x;
    float y;
};

/*
 * Take an angle in degrees (counter-clockwise from origin),
 * convert it to a unit vector direction.
 * (ex) 75 degrees => (0.2588, 0.966)
 *
 *     15d
 *     /|
 * c=1/ |
 *   /  | y
 *  /___|
 * 75d  90d
 *    x
 *
 * law of cosines
 * y = sqrt(x^2 + c^2 - 2xc*cos75)
 * y = sqrt(x^2 + 1 - 2x*cos75)
 * y = sqrt(0.268^2 + 1 - 2(0.268)cos75) = 0.966
 *
 * law of sines
 * x = c sin(a)/sin(b)
 * x = 1 sin(15)/sin(90) = 0.2588
 */
static vec2 get_unit_vector_from_angle_degrees(float angle) {
    float radians = angle * DEGREES_TO_RADIANS;
    float radians90 = 90 * DEGREES_TO_RADIANS;
    float x = 1 * (float)sinf(radians90-radians) / (float)sinf(radians90);
    float y = sqrtf(powf(x, 2) + 1 - 2*x*1*cosf(radians));
    return vec2{
            x, y
    };
}

static int getRandomInt(int start, int end) {
    return start + (rand() % (end - start + 1));
}

static float getRandomFloat(float start, float end) {
    return start + static_cast<float>(rand()) / RAND_MAX * (end - start);
}

static vec2 normalized_to_screen(vec2 vec) {
    // Convert normalized coordinates (-1 to 1) to screen coordinates (0 to screenWidth/Height)
    float screenX = (vec.x + 1) * 0.5f * WINDOW_WIDTH;
    float screenY = (1 - vec.y) * 0.5f * WINDOW_HEIGHT;
    return {screenX, screenY};
}
static vec2 screen_to_normalized(vec2 vec) {
    // Convert screen coordinates (0 to screenWidth/Height) to normalized coordinates (-1 to 1)
    float deviceX = (2 * vec.x) / WINDOW_WIDTH - 1;
    float deviceY = 1 - (2 * vec.y) / WINDOW_HEIGHT;
    return {deviceX, deviceY};
}
static vec3 normalized_to_screen(vec3 vec) {
    // Convert normalized coordinates (-1 to 1) to screen coordinates (0 to screenWidth/Height)
    float screenX = (vec.x + 1) * 0.5f * WINDOW_WIDTH;
    float screenY = (1 - vec.y) * 0.5f * WINDOW_HEIGHT;
    return {screenX, screenY,0};
}
static vec3 screen_to_normalized(vec3 vec) {
    // Convert screen coordinates (0 to screenWidth/Height) to normalized coordinates (-1 to 1)
    float deviceX = (2 * vec.x) / WINDOW_WIDTH - 1;
    float deviceY = 1 - (2 * vec.y) / WINDOW_HEIGHT;
    return {deviceX, deviceY,0};
}

static vec2 rotate_euler(vec2 transform, float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    float rotation_matrix[4] = { cos, -sin, sin, cos };
    return {
            (transform.x*rotation_matrix[0] + transform.y*rotation_matrix[1]),
            (transform.x*rotation_matrix[2] + transform.y*rotation_matrix[3]),
    };
}
static vec3 rotate_euler(vec3 transform, float angle) {
    float radians = angle * PI/180.0f;
    float cos = cosf(radians);
    float sin = sinf(radians);
    float rotation_matrix[4] = { cos, -sin, sin, cos };
    return {
            (transform.x*rotation_matrix[0] + transform.y*rotation_matrix[1]),
            (transform.x*rotation_matrix[2] + transform.y*rotation_matrix[3]),
            0,
    };
}

static vec3 scale_vector(vec3 transform, vec3 scale) {
    return {
        transform.x*scale.x, transform.y*scale.y, transform.z*scale.z
    };
}

static void rotate_vertices(vec3* vertices, int vertexCount, vec3 pos, float degrees) {
    vec3 posPixels = normalized_to_screen(pos);
    for (int i = 0; i < vertexCount; i ++) {
        // "Undo" current position transform back to screen space origin (top left 0,0)
        vec3 vertex_pixels = normalized_to_screen(vertices[i]);
        vec3 newVertex = {vertex_pixels.x - posPixels.x, vertex_pixels.y - posPixels.y, 0};

        // Rotate
        newVertex = rotate_euler(newVertex, degrees);

        // "Redo" position back
        vertices[i] = screen_to_normalized({newVertex.x + posPixels.x, newVertex.y + posPixels.y, 0});
    }
}

static void translate_vertices(vec3* vertices, int vertexCount, vec3 translate) {
    for (int i = 0; i < vertexCount; i ++) {
        vertices[i].x += translate.x;
        vertices[i].y += translate.y;
        vertices[i].z += translate.z;
    }
}

#endif //GAMEENGINE_VECTOR_H
