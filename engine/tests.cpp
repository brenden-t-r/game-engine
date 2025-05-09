#include "engine/vector.h"

#include <cassert>
#include <cstdio>

static void tests_getUnitVectorFromAngleDegrees() {
    vec2 vec = get_unit_vector_from_angle_degrees(45);
    assert(fabsf(vec.x - 0.707f) < 0.001f);
    assert(fabsf(vec.y - 0.707f) < 0.001f);
    vec = get_unit_vector_from_angle_degrees(75);
    assert(fabsf(vec.x - 0.259f) < 0.001f);
    assert(fabsf(vec.y - 0.966f) < 0.001f);
}

int main() {
    printf("Running Tests");
    tests_getUnitVectorFromAngleDegrees();
    return 0;
}
