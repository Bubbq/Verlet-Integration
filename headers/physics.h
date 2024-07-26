#ifndef VPHYSICS_H_
#define VPHYSICS_H_

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

// void handle_verlet_circle_collision(VerletCirlce*, VerletCirlce*);
// void handle_border_collision(VerletCirlce*, Vector2, float);
void apply_gravity(VerletCirlce*, Vector2);
void update_position(VerletCirlce*,float);

typedef struct
{
    int size;
    size_t capacity;
    VerletCirlce* circle;
} Circles;

void add_verlet_circle(Circles*,VerletCirlce);
// void remove_verlet_circle(Circles*,int);
void resize_circles(Circles*);

typedef struct
{
    VerletCirlce* circle1;
    VerletCirlce* circle2;
    float target_distance;
} Link;

void maintain_link(Link*);

typedef struct
{
	int size;
	Link* link;
	size_t capacity;
} Chain;

void resize_chain(Chain*);
void add_link(Chain*,Link);
void remove_link(Chain*,int);

#endif