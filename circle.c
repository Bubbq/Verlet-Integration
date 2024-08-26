#include "headers/circle.h"
#include "headers/raylib.h"

Circles create_circles()
{
	Circles circles;

	circles.size = 0;
	circles.capacity = sizeof(VerletCirlce);
	circles.circle = malloc(circles.capacity);

	return circles;
}

void draw_circles(Circles* circles)
{
	int c = 0;
	
	for(VerletCirlce* vc = circles->circle; c < circles->size; c++, vc = (circles->circle + c))
		DrawCircleSector(vc->current_position, vc->radius, 0, 360, 1, vc->color);
}

void resize_circles(Circles* circles)
{   
	circles->capacity *= 2;
	circles->circle = realloc(circles->circle, circles->capacity);
}

void delete_verlet_circle(Circles* circles, int position)
{
	for(int i = position; i < circles->size; i++)
		circles->circle[i] = circles->circle[i + 1];

	circles->size--;
}

void add_verlet_circle(Circles* circles, VerletCirlce circle)
{
	if((circles->size * sizeof(VerletCirlce)) == circles->capacity)
		resize_circles(circles);

	circles->circle[circles->size++] = circle;
}