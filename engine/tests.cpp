#include "engine/vector.h"
#include "engine/gameobject.h"

#include <cassert>
#include <cstdio>
#include <cmath>

bool equalsFloat(float expected, float actual) {
    return std::fabs(expected - actual) < 0.001;
}

bool equalsMatrix(Matrix3 expected, Matrix3 actual) {
    if (!equalsFloat(expected._11, actual._11)) return false;
    if (!equalsFloat(expected._12, actual._12)) return false;
    if (!equalsFloat(expected._13, actual._13)) return false;
    if (!equalsFloat(expected._21, actual._21)) return false;
    if (!equalsFloat(expected._22, actual._22)) return false;
    if (!equalsFloat(expected._23, actual._23)) return false;
    if (!equalsFloat(expected._31, actual._31)) return false;
    if (!equalsFloat(expected._32, actual._32)) return false;
    if (!equalsFloat(expected._33, actual._33)) return false;
    return true;
}

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
    // Scaling x and y equally
    vec3 point = {-0.5, -0.5, 0};
    vec3 pos = {0, 0, 0};
    vec3 scale = {2, 2, 1};
    vec3 rot = {0, 0, 0};
    Matrix3 result = matrix_transformation(pos, scale, rot);
    vec3 transformed = matrix_multiply_vec(result, point);
    assert(transformed.x == -1);
    assert(transformed.y == -1);

    // Scaling x only
    point = {-0.5, -0.5, 0};
    pos = {0, 0, 0};
    scale = {2, 1, 1};
    rot = {0, 0, 0};
    result = matrix_transformation(pos, scale, rot);
    transformed = matrix_multiply_vec(result, point);
    assert(transformed.x == -1);
    assert(transformed.y == -0.5);

    // Translation + scale
    point = {-0.5, -0.5, 1};
    pos = {0.7, 1, 1};
    scale = {2, 1, 1};
    rot = {0, 0, 0};
    result = matrix_transformation(pos, scale, rot);
    transformed = matrix_multiply_vec(result, point);
    assert(equalsFloat(transformed.x, -0.3f) == true);
    assert(equalsFloat(transformed.y, 0.5f) == true);

    // Rotation
    point = {-0.5, -0, 0};
    pos = {0, 0, 0};
    scale = {1, 1, 1};
    rot = {0, 0, 180};
    result = matrix_transformation(pos, scale, rot);
    transformed = matrix_multiply_vec(result, point);
    assert(equalsFloat(transformed.x, 0.5) == true);
    assert(equalsFloat(transformed.y, 0) == true);
}

static void tests_normalizedToScreen() {
    vec3 point {0, 0, 1};
    vec3 a = normalized_to_screen(point);
    vec3 b = normalized_to_screen_using_matrix(point);
    assert(a.x == 1920.0/2);
    assert(a.y == 1080.0/2);
    assert(a.x == b.x);
    assert(a.y == b.y);
}

static void tests_screenToNormalized() {
    vec3 point {1920.0/2, 1080.0/2, 1};
    vec3 a = screen_to_normalized(point);
    vec3 b = screen_to_normalized_using_matrix(point);
    assert(a.x == 0);
    assert(a.y == 0);
    assert(a.x == b.x);
    assert(a.y == b.y);
}

static void tests_transpose() {
    Matrix3 M = {
            0, 4, -2,
            1, 1, -1,
            -2, -8, 4
    };
    Matrix3 expected = {
            0, 1, -2,
            4, 1, -8,
            -2, -1, 4
    };
    Matrix3 actual = transpose(M);
    assert(equalsMatrix(expected, actual) == true);
}

static void tests_determinant() {
    Matrix3 M = {
            2, 1, 3,
            0, 2, 4,
            1, 1, 2
    };
    float expected = -2;
    float actual = determinant(M);
    assert(equalsFloat(expected, actual) == true);
}

static void tests_adjoint() {
    Matrix3 M = {
            3, 0, 2,
            2, 1, 0,
            1, 4, 2
    };
    Matrix3 expected = {
            2, 8, -2,
            -4, 4, 4,
            7, -12, 3
    };
    Matrix3 actual = adjoint(M);
    assert(equalsMatrix(expected, actual) == true);
}

static void tests_inverse() {
    Matrix3 M = {
            3, 0, 2,
            2, 1, 0,
            1, 4, 2
    };
    Matrix3 expected = {
            1.0/10, 2.0/5, -1.0/10,
            -1.0/5, 1.0/5, 1.0/5,
            7.0/20, -3.0/5, 3.0/20
    };
    Matrix3 actual = matrix_inverse(M);
    assert(equalsMatrix(expected, actual) == true);
}

int main() {
    printf("Running Tests\n");
    tests_getUnitVectorFromAngleDegrees();
    tests_dotProduct();
    tests_matrixMultiply();
    tests_matrixMultiplyVec();
    tests_matrixTransformation();
    tests_normalizedToScreen();
    tests_screenToNormalized();
    tests_transpose();
    tests_determinant();
    tests_adjoint();
    tests_inverse();
    return 0;
}
