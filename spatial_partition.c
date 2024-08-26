#include "headers/spatial_partition.h"
#include "headers/raylib.h"

void resize_index_list(IndexList* il)
{
	il->capacity *= 2;
	il->indicies = realloc(il->indicies, il->capacity);
}

void add_circle_index(IndexList* il, int index)
{
	if((il->size * sizeof(int)) == il->capacity) 
		resize_index_list(il);
	
	il->indicies[il->size++] = index;
}

void create_grid(Grid grid[ROW][COL], Vector2 border_center)
{
	Vector2 current_position = { (border_center.x - BORDER_RADIUS), (border_center.y - BORDER_RADIUS) };

	for(int r = 0; r < ROW; r++, current_position = (Vector2){(border_center.x - BORDER_RADIUS), (current_position.y + CSIZE)})
		for(int c = 0; c < COL; c++, current_position.x += CSIZE)
		{
			Cell cell;
			cell.start = current_position;
			cell.index_list.size = 0;
			cell.index_list.capacity = sizeof(int);
			cell.index_list.indicies = malloc(cell.index_list.capacity);

			grid[r][c] = cell;
		}
}

void add_circle_to_grid(Grid grid[ROW][COL], int c_index, Vector2 position)
{
	int c = ((position.x - grid[0][0].start.x) / CSIZE);
	int r = ((position.y - grid[0][0].start.y) / CSIZE);
	
	if((r < ROW) && (c < COL)) 
		add_circle_index(&grid[r][c].index_list, c_index);
}

void clear_grid_index_lists(Grid grid[ROW][COL])
{
	for(int r = 0; r < ROW; r++) 
		for(int c = 0; c < COL; c++) 
			grid[r][c].index_list.size = 0;
}

void grid_circle_collision(Grid grid[ROW][COL], Circles* circles)
{
	for (int r = 0; r < ROW; r++)
		for (int c = 0; c < COL; c++)
		{
			// same cell circle coll.
			IndexList* il = &grid[r][c].index_list;

			for (int i = 0; i < il->size; i++) 
				for (int j = i + 1; j < il->size; j++) 
					handle_verlet_circle_collision(&circles->circle[il->indicies[i]], &circles->circle[il->indicies[j]]);

			// neighbor cell circle collisions
			for (int dx = -1; dx <= 1; dx++)
				for (int dy = -1; dy <= 1; dy++)
				{
					int nr = r + dx;
					int nc = c + dy;
					IndexList* nil = &grid[nr][nc].index_list;
					// out of bounds case
					if ((nr < 0 || nr >= ROW) || (nc < 0 || nc >= ROW) || (nr == r && nc == c)) continue;
					
					for (int i = 0; i < il->size; i++)
						for (int j = 0; j < nil->size; j++)
							handle_verlet_circle_collision(&circles->circle[il->indicies[i]], &circles->circle[nil->indicies[j]]);
				}
		}
}

void dealloc_grid(Grid grid[ROW][COL])
{
	for(int r = 0; r < ROW; r++) 
		for(int c = 0; c < COL; c++)
			free(grid[r][c].index_list.indicies);
}