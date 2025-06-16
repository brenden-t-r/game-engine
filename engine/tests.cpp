#include "engine/vector.h"
#include "engine/gameobject.h"

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

static void tests_dotProduct() {
    vec3 a = {-6, 8, 0};
    vec3 b = {5, 12, 0};
    assert(66 == dot(a, b));
    a = {-12, 16, 0};
    b = {12, 9, 0};
    assert(0 == dot(a, b));
}

static void tests_matrixMultiply() {
    Matrix3 a = {
            -2, 3, 4,
            2, -3, 5,
            0, 3, -4
    };
    Matrix3 b = {
            4, 2, -2,
            1, 4, 3,
            2, 5, 3
    };
    Matrix3 quotient = matrix_multiply(a, b);
    assert(quotient.Row(0).x == 3);
    assert(quotient.Row(0).y == 28);
    assert(quotient.Row(0).z == 25);
    assert(quotient.Row(1).x == 15);
    assert(quotient.Row(1).y == 17);
    assert(quotient.Row(1).z == 2);
    assert(quotient.Row(2).x == -5);
    assert(quotient.Row(2).y == -8);
    assert(quotient.Row(2).z == -3);
}

static void tests_matrixMultiplyVec() {
    Matrix3 mat = {
            1, 2, 3,
            4, 5, 6,
            7, 8, 9
    };
    vec3 vec = {
            2, 1, 3
    };
    vec3 quotient = matrix_multiply_vec(mat, vec);
    assert(quotient.x == 13);
    assert(quotient.y == 31);
    assert(quotient.z == 49);
}

static void tests_matrixTransformation() {
    vec3 point  {1, 1, 0};
    vec3 pos = {2, 3, 0};
    vec3 scale = {2, 1, 1};
    vec3 rot = {0, 0, 0.5235988};
    Matrix3 result = matrix_transformation(pos, scale, rot);
    vec3 transformed = matrix_multiply_vec(result, point);
    assert(transformed.x == 3.732);
    assert(transformed.y == 4.5);
}

int main() {
    printf("Running Tests\n");
    tests_getUnitVectorFromAngleDegrees();
    tests_dotProduct();
    tests_matrixMultiply();
    tests_matrixMultiplyVec();
    tests_matrixTransformation();
    return 0;
}
