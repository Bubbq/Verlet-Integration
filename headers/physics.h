#ifndef VERLET_PHYSICS_H_
#define VERLET_PHYSICS_H_

#include "raylib.h"
#include <stdlib.h>

typedef enum
{
    FREE = 0,
    SUSPENDED = 1,
} Status;

typedef struct
{
    Color color;
    float radius;
    Status status;
    Vector2 acceleration;
    Vector2 current_position;
    Vector2 previous_position;
} VerletCirlce;

void handle_border_collision(VerletCirlce* circle, Vector2 constraint_center, Vector2 world_gravity, float constraint_radius);
void handle_verlet_circle_collision(VerletCirlce* circle1, VerletCirlce* circle2);
void update_position(VerletCirlce* circle, float slow_down_scale, float dt);
void apply_gravity(VerletCirlce* circle, Vector2 world_gravity, float dt);

typedef struct
{
    int size;
    size_t capacity;
    VerletCirlce* circle;
} Circles;

void add_verlet_circle(Circles* circles, VerletCirlce circle);
void remove_verlet_circle(Circles* circles, int position);
void resize_circles(Circles* circles);

typedef struct
{
    VerletCirlce* circle1;
    VerletCirlce* circle2;
    float target_distance;
} Link;

void maintain_link(Link* link);

typedef struct
{
	int size;
	Link* link;
	size_t capacity;
} Chain;

void add_link(Chain* chain, Link link);
void remove_link(Chain* chain, int position);
void resize_chain(Chain* chain);

#endif