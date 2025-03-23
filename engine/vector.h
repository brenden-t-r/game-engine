#ifndef GAMEENGINE_VECTOR_H
#define GAMEENGINE_VECTOR_H

#include "cmath"

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

static vector2 coords_device_to_screen(vector2 vec) {
    // Convert normalized coordinates (-1 to 1) to screen coordinates (0 to screenWidth/Height)
    float screenX = (vec.x + 1) * 0.5f * WINDOW_WIDTH;
    float screenY = (1 - vec.y) * 0.5f * WINDOW_HEIGHT;
    return {screenX, screenY};
}
static vector2 coords_screen_to_device(vector2 vec) {
    // Convert screen coordinates (0 to screenWidth/Height) to normalized coordinates (-1 to 1)
    float deviceX = (2 * vec.x) / WINDOW_WIDTH - 1;
    float deviceY = 1 - (2 * vec.y) / WINDOW_HEIGHT;
    return {deviceX, deviceY};
}
static vector3 coords_device_to_screen(vector3 vec) {
    // Convert normalized coordinates (-1 to 1) to screen coordinates (0 to screenWidth/Height)
    float screenX = (vec.x + 1) * 0.5f * WINDOW_WIDTH;
    float screenY = (1 - vec.y) * 0.5f * WINDOW_HEIGHT;
    return {screenX, screenY,0};
}
static vector3 coords_screen_to_device(vector3 vec) {
    // Convert screen coordinates (0 to screenWidth/Height) to normalized coordinates (-1 to 1)
    float deviceX = (2 * vec.x) / WINDOW_WIDTH - 1;
    float deviceY = 1 - (2 * vec.y) / WINDOW_HEIGHT;
    return {deviceX, deviceY,0};
}

static vector2 rotate_euler(vector2 transform, float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    float rotation_matrix[4] = { cos, -sin, sin, cos };
    return {
        (transform.x*rotation_matrix[0] + transform.y*rotation_matrix[1]),
        (transform.x*rotation_matrix[2] + transform.y*rotation_matrix[3]),
    };
}
static vector3 rotate_euler(vector3 transform, float angle) {
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

#endif //GAMEENGINE_VECTOR_H
