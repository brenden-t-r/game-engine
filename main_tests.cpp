#include "engine/vector.h"

#include <cassert>

static void tests_getUnitVectorFromAngleDegrees() {
    engine_vector2 vec = get_unit_vector_from_angle_degrees(45);
    assert(fabsf(vec.x - 0.707f) < 0.001f);
    assert(fabsf(vec.y - 0.707f) < 0.001f);
    vec = get_unit_vector_from_angle_degrees(75);
    assert(fabsf(vec.x - 0.259f) < 0.001f);
    assert(fabsf(vec.y - 0.966f) < 0.001f);
}

int main() {
    tests_getUnitVectorFromAngleDegrees();
    return 0;
}
