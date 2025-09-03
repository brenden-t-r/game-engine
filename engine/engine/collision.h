/**
 * 2D Collision detection
 *
 * Supports:
 * AABB box vs AABB box
 * OOBB box vs point
 * OOBB box vs OOBB box
 *
 *  Pivot
 *  TOP_LEFT | Collider is positioned at vector2 `pos` relative to top-left
 *  CENTER   | Collider is centered, with `pos` translation from center.
 */
#ifndef GAMEENGINE_COLLISION_H
#define GAMEENGINE_COLLISION_H

#include "../platform/platform.h"
#include "vector.h"
#include "gameobject.h"

enum PIVOT {
    TOP_LEFT = 0,
    CENTER = 1
};

struct box_collider {
    struct vec2 pos;
    float width;
    float height;
    enum PIVOT pivot;
};

//region AABB
static int AABB_collision(
        struct vec2 p1, struct vec2 p2,
        struct box_collider c1, struct box_collider c2
) {
    float c1x = p1.x + c1.pos.x;
    float c1y = p1.y + c1.pos.y;
    float c2x = p2.x + c2.pos.x;
    float c2y = p2.y + c2.pos.y;

    if (c1.pivot == CENTER) {
        c1x -= c1.width / 2;
        c1y += c1.height / 2;
    }
    if (c2.pivot == CENTER) {
        c2x -= c2.width / 2;
        c2y += c2.height / 2;
    }

    bool x_overlap = (c1x >= c2x && c1x <= c2x + c2.width) || (c1x < c2x && c1x + c1.width > c2x);
    bool y_overlap = (c1y >= c2y && c1y - c1.height <= c2y) || (c1y <= c2y && c1y >= c2y - c2.height);
    if (x_overlap && y_overlap) {
        // We have a collision, folks.
        return 1;
    }

    return 0;
}

static int AABB_collision(
        struct vec2 p1, struct vec2 p2,
        struct box_collider c1
) {
    float c1x = p1.x + c1.pos.x;
    float c1y = p1.y + c1.pos.y;

    if (c1.pivot == CENTER) {
        c1x -= c1.width / 2;
        c1y += c1.height / 2;
    }

    bool x_overlap = p2.x > c1x && p2.x < (c1x + c1.width);
    bool y_overlap = p2.y < c1y && p2.y > (c1y - c1.height);
    if (x_overlap && y_overlap) {
        // We have a collision, folks.
        return 1;
    }

    return 0;
}

static int AABB_collision(
        struct vec3 p1, struct vec3 p2,
        struct box_collider c1, struct box_collider c2
) {
    return AABB_collision(vec2{p1.x, p1.y}, vec2{p2.x, p2.y}, c1, c2);
}

static int AABB_collision(
        struct vec3 p1, struct vec3 p2,
        struct box_collider c1
) {
    return AABB_collision(vec2{p1.x, p1.y}, vec2{p2.x, p2.y}, c1);
}

static bool AABB_collision(GameObject* a, GameObject* b) {
    auto obj1_collider = box_collider{{0, 0}, a->transform.width, a->transform.height, PIVOT::CENTER};
    auto obj2_collider = box_collider{{0, 0}, b->transform.width,b->transform.height, PIVOT::CENTER};
    return AABB_collision(a->transform.pos, b->transform.pos, obj1_collider, obj2_collider);
}

static bool AABB_collision(GameObject* a, vec3 point) {
    auto obj1_collider = box_collider{{0, 0}, a->transform.width, a->transform.height, PIVOT::CENTER};
    return AABB_collision(a->transform.pos, point, obj1_collider);
}

struct aabb_hit_edge {
    bool left;
    bool right;
    bool top;
    bool bottom;
};

// Mosty works, but doesn't take into account velocity, so has some edge cases
static aabb_hit_edge aabb_get_hit_edge(GameObject* a, GameObject* b) {
    vec3 aPos = a->transform.pos;
    vec3 bPos = b->transform.pos;
    float b_x_left = (bPos.x - b->transform.width / 2);
    float b_x_right = (bPos.x + b->transform.width / 2);
    float b_y_top = (bPos.y + b->transform.height / 2);
    float b_y_bottom = (bPos.y - b->transform.height / 2);
    float a_x_left = (aPos.x - a->transform.width / 2);
    float a_x_right = (aPos.x + a->transform.width / 2);
    float a_y_top = (aPos.y + a->transform.height / 2);
    float a_y_bottom = (aPos.y - a->transform.height / 2);

    float difference_l = a_x_right - b_x_left;
    float difference_r = b_x_right - a_x_left;
    float difference_b = a_y_top - b_y_bottom;
    float difference_t = b_y_top - a_y_bottom;

    if (difference_l < 0) difference_l = 1;
    if (difference_r < 0) difference_l = 1;
    if (difference_b < 0) difference_b = 1;
    if (difference_t < 0) difference_t = 1;

    bool hitFromLeft = (difference_l <= difference_b) && (difference_l <= difference_t) && (difference_l <= difference_r);
    bool hitFromRight = (difference_r <= difference_b) && (difference_r <= difference_t) && (difference_r <= difference_l);
    bool hitFromBottom = (difference_b <= difference_l) && (difference_b <= difference_t) && (difference_b <= difference_r);
    bool hitFromTop = (difference_t <= difference_l) && (difference_t <= difference_b) && (difference_t <= difference_r);
    return {
        hitFromLeft, hitFromRight, hitFromTop, hitFromBottom
    };
}
//endregion

//region OOBB
struct OOBB_box_collider {
    vec2 center;
    vec2 halfExtents; // half-widths along local axes
    float rotation;   // radians
#ifdef DEBUG_COLLIDERS
    GameObject* debug[4];
#endif
};

OOBB_box_collider get_OOBB_box_collider(Sprite* sprite) {
    vec3 sumOfVertices =
            normalized_to_screen(sprite->vertices[0]) +
            normalized_to_screen(sprite->vertices[1]) +
            normalized_to_screen(sprite->vertices[2]) +
            normalized_to_screen(sprite->vertices[3]);

    float centerX = sumOfVertices.x * 0.25f;
    float centerY = sumOfVertices.y * 0.25f;

    // Multiply by half the window width since NDC width/height is 2. Multiply by half again to get half width.
    float halfWidth  = sprite->transform.width  * WINDOW_WIDTH  * 0.25f * sprite->transform.scale.x;
    float halfHeight = sprite->transform.height * WINDOW_HEIGHT * 0.25f * sprite->transform.scale.y;

    return OOBB_box_collider {
            vec2{centerX, centerY},
            vec2{halfWidth, halfHeight},
            sprite->transform.rot.z
    };
}

// Get corners of OOBB in world space
void get_OOBB_corners(const OOBB_box_collider& obb, vec2 outCorners[4]) {
    vec2 xAxis = rotate_euler(vec2{obb.halfExtents.x, 0}, obb.rotation);
    vec2 yAxis = rotate_euler(vec2{0, obb.halfExtents.y}, obb.rotation);
    outCorners[0] = obb.center + xAxis + yAxis;
    outCorners[1] = obb.center - xAxis + yAxis;
    outCorners[2] = obb.center - xAxis - yAxis;
    outCorners[3] = obb.center + xAxis - yAxis;
}

void project_onto_axis(const vec2 corners[4], const vec2& axis, float& min, float& max) {
    min = max = corners[0].x * axis.x + corners[0].y * axis.y;
    for(int i=1;i<4;i++){
        float proj = corners[i].x * axis.x + corners[i].y * axis.y;
        if(proj < min) min = proj;
        if(proj > max) max = proj;
    }
}

bool OOBB_collision(const OOBB_box_collider& a, const OOBB_box_collider& b) {
    vec2 ca[4];
    vec2 cb[4];
    get_OOBB_corners(a, ca);
    get_OOBB_corners(b, cb);
    vec2 axes[4] = {
            rotate_euler(vec2{1,0}, a.rotation),
            rotate_euler(vec2{0,1}, a.rotation),
            rotate_euler(vec2{1,0}, b.rotation),
            rotate_euler(vec2{0,1}, b.rotation)
    };

    for(int i = 0; i < 4; i++) {
        float minA, maxA, minB, maxB;
        project_onto_axis(ca, axes[i], minA, maxA);
        project_onto_axis(cb, axes[i], minB, maxB);
        if(maxA < minB || maxB < minA) {
            return false; // separating axis found
        }
    }
    // We have a collision, folks
    return true;
}

bool OOBB_collision(const OOBB_box_collider& box, const vec2& point) {
    vec2 pointPx = normalized_to_screen(vec2{point.x, point.y});
    vec2 translatedPoint = pointPx - box.center;
    vec2 local = rotate_euler(translatedPoint, -box.rotation);

    // Check against box extents
    float hx = box.halfExtents.x;
    float hy = box.halfExtents.y;
    return (local.x >= -hx && local.x <= hx &&
            local.y >= -hy && local.y <= hy);
}

bool OOBB_collision(Sprite* a, vec3 point) {
    OOBB_box_collider collider = get_OOBB_box_collider(a);
    return OOBB_collision(collider, vec2{point.x, point.y});
}

#ifdef DEBUG_COLLIDERS
void debug_OOBB_collider(Platform* platform, OOBB_box_collider &oobb) {
    vec2 corners[4];
    get_OOBB_corners(oobb, corners);
    for (int i = 0; i < 4; i++) {
        if (oobb.debug[i] == nullptr) {
            GameObject* tri = platform->CreateTriangle();
            tri->transform.width = 0.02f;
            tri->transform.height = 0.02f;
            oobb.debug[i] = tri;
        }
        oobb.debug[i]->transform.pos = screen_to_normalized({corners[i].x, corners[i].y, 0});
        oobb.debug[i]->Update();
    }
}
#endif
//endregion

#endif // GAMEENGINE_COLLISION_H