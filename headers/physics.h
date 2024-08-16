#ifndef VERLET_PHYSICS_H_
#define VERLET_PHYSICS_H_

#include <stdlib.h>
#include "raylib.h"
#include "circle.h"
#include "link.h"

void handle_verlet_circle_collision(VerletCirlce* circle1, VerletCirlce* circle2);
void update_position(VerletCirlce* circle, float slow_down_scale, float dt);
void apply_gravity(VerletCirlce* circle, Vector2 world_gravity, float dt);
void maintain_link(Link* link);

#endif