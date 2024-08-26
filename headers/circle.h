#ifndef CIRCLE_H
#define CIRCLE_H

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

typedef struct
{
	int size;
	size_t capacity;
	VerletCirlce* circle;
} Circles;

Circles create_circles();
void draw_circles(Circles* circles);
void resize_circles(Circles* circles);
void delete_verlet_circle(Circles* circles, int position);
void add_verlet_circle(Circles* circles, VerletCirlce circle);

#endif