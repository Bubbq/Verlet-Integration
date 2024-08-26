#ifndef SP_H
#define SP_H

#include <stdlib.h>
#include "raylib.h"
#include "circle.h"
#include "physics.h"

// for optimal preformance, let the size of a cell be the diameter of the balls you make
static const int CSIZE = 20;
static const float BORDER_RADIUS = 400.0;
static const int ROW = ((BORDER_RADIUS * 2) / CSIZE), COL = ((BORDER_RADIUS * 2) / CSIZE);

typedef struct
{
	int size;
	int* indicies;
	size_t capacity;
} IndexList;

void resize_index_list(IndexList* il);
void add_circle_index(IndexList* il, int index);

typedef struct 
{
	Vector2 start;
	IndexList index_list;
} Grid;
typedef Grid Cell;

void create_grid(Grid grid[ROW][COL], Vector2 border_center);
void add_circle_to_grid(Grid grid[ROW][COL], int c_index, Vector2 position);
void clear_grid_index_lists(Grid grid[ROW][COL]);
void grid_circle_collision(Grid grid[ROW][COL], Circles* circles);
void dealloc_grid(Grid grid[ROW][COL]);

#endif