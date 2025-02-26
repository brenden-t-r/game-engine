#ifndef GAMEENGINE_VECTOR_H
#define GAMEENGINE_VECTOR_H

#include "cmath"

constexpr float PI = 3.14159265358979323846f;
constexpr float DEGREES_TO_RADIANS = PI/180;

struct engine_vector3 {
    float x;
    float y;
    float z;
};
struct engine_vector2 {
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
static engine_vector2 get_unit_vector_from_angle_degrees(float angle) {
    float radians = angle * DEGREES_TO_RADIANS;
    float radians90 = 90 * DEGREES_TO_RADIANS;
    float x = 1 * (float)sinf(radians90-radians) / (float)sinf(radians90);
    float y = sqrtf(powf(x, 2) + 1 - 2*x*1*cosf(radians));
    return engine_vector2{
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
