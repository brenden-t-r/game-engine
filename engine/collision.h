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

enum PIVOT {
    TOP_LEFT = 0,
    CENTER = 1
};

struct box_collider {
    struct engine_vector2 pos;
    float width;
    float height;
    enum PIVOT pivot;
};

static int AABB_collision(
        struct engine_vector2 p1, struct engine_vector2 p2,
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
        struct engine_vector3 p1, struct engine_vector3 p2,
        struct box_collider c1, struct box_collider c2
) {
    return AABB_collision(engine_vector2{p1.x, p1.y}, engine_vector2{p2.x, p2.y}, c1, c2);
}

static bool AABB_collision(GameObject* a, GameObject* b) {
    auto obj1_collider = box_collider{{0, 0}, a->transform.width, a->transform.height, PIVOT::CENTER};
    auto obj2_collider = box_collider{{0, 0}, b->transform.width,b->transform.height, PIVOT::CENTER};
    return AABB_collision(a->transform.pos, b->transform.pos, obj1_collider, obj2_collider);
}

#endif // GAMEENGINE_COLLISION_H