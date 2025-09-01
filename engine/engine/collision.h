/**
 * 2D Collision detection
 *
 * Supports box colliders, in upright orientation (no rotation)
 *
 *  Pivot
 *  TOP_LEFT | Collider is positioned at vector2 `pos` relative to top-left
 *  CENTER   | Collider is centered, with `pos` translation from center.
 */
#ifndef GAMEENGINE_COLLISION_H
#define GAMEENGINE_COLLISION_H

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
        struct vec3 p1, struct vec3 p2,
        struct box_collider c1, struct box_collider c2
) {
    return AABB_collision(vec2{p1.x, p1.y}, vec2{p2.x, p2.y}, c1, c2);
}

static bool AABB_collision(GameObject* a, GameObject* b) {
    auto obj1_collider = box_collider{{0, 0}, a->transform.width, a->transform.height, PIVOT::CENTER};
    auto obj2_collider = box_collider{{0, 0}, b->transform.width,b->transform.height, PIVOT::CENTER};
    return AABB_collision(a->transform.pos, b->transform.pos, obj1_collider, obj2_collider);
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

#endif // GAMEENGINE_COLLISION_H