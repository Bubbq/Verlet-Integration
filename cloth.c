#include "headers/raylib.h"
#include "headers/raymath.h"
#include "headers/physics.h"
#include "headers/circle.h"
#include "headers/link.h"
#include <stdlib.h>

const int SCRW = 900, SCRH = 900;
const int FPS = 60;

const int ROW = 50, COL = 50;

const int XPAD = 100;
const int YPAD = 30;

const int XDIST = ((SCRW - (2 * XPAD)) / (COL - 1));
const int YDIST = 1;

// directly affects tension of links
const int SUB_STEPS = 5;

const Vector2 WORLD_GRAVITY = { 0, 2000.0f };

void update_circles(Circles* circles, int* grabbed_link_pos)
{
	// slowdown scale factor
	const float DAMP = 0.975f;

	int i = 0;
	for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i))
	{
		if(CheckCollisionPointCircle(GetMousePosition(), vc->current_position, vc->radius) && !(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) && (i > ROW))
			*grabbed_link_pos = i;
		
		if(vc->status == FREE)
		{
			apply_gravity(vc, WORLD_GRAVITY, GetFrameTime());
			update_position(vc, DAMP, GetFrameTime());
		}
	}
}

void update_links(Chain* chain)
{
	const float MAX_LINK_DIST = 100.0f;

	int l = 0;
	for(Link* link = chain->link; l < chain->size; l++, link = (chain->link + l))
	{
		Vector2 starting_position = link->circle1->current_position, 
				ending_position = link->circle2->current_position;
		
		if((Vector2Distance(starting_position, ending_position) >= MAX_LINK_DIST) || ((IsMouseButtonDown(MOUSE_BUTTON_LEFT)) && (CheckCollisionPointLine(GetMousePosition(), starting_position, ending_position, 5))))
			delete_link(chain, l);
		
		maintain_link(link);
	}   
}

void init_circles(Circles* circles)
{
	const int RADIUS = 5;
 
	Vector2 vc_position = { XPAD, YPAD };
	
	for(int r = 0; r < ROW; r++, vc_position = (Vector2){ XPAD, (YDIST * r) })
		for(int c = 0; c < COL; (vc_position.x += XDIST), c++)
		{
			VerletCirlce verlet_circle;

			verlet_circle.color = LIGHTGRAY;
			verlet_circle.radius = RADIUS;
			verlet_circle.status = (r == 0) ? SUSPENDED : FREE;
			verlet_circle.acceleration = (Vector2){ 0 };
			verlet_circle.current_position = verlet_circle.previous_position = vc_position;

			add_verlet_circle(circles, verlet_circle);
		}
}

void init_chain(Chain* chain, Circles* circles)
{
	for(int r = 0; r < ROW; r++)
	{
		for(int c = 0; c < COL; c++)
		{
			// one dimensional index representation of  2d array
			int i_index = (r * ROW) + c;

			for(int dx = -1; dx <= 1; dx++)
			{
				for(int dy = -1; dy <= 1; dy++)
				{
					// neighboring row and columns
					int nr = (r + dx);
					int nc = (c + dy);
					int j_index = (nr * ROW) + nc; 
					
					if((nr >= ROW || nr < 0) || (nc >= COL || nc < 0) || ((nr == r) && (nc == c))) 
						continue;

					Link link;

					link.circle1 = (circles->circle + i_index);
					link.circle2 = (circles->circle + j_index);

					if(r == nr)
					{
						link.target_distance = XDIST;
						add_link(chain, link);
					}

					else if(c == nc)
					{
						link.target_distance = (SCRW - (2 * YPAD)) / (ROW - 1.0f);
						add_link(chain, link);
					}
				}
			}
		}
	}
}

void pull_cloth(Circles* circles, int index)
{
	if((index != -1) && (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))) 
		circles->circle[index].current_position = GetMousePosition(); 
}

void init()
{
	SetTargetFPS(FPS);
	SetTraceLogLevel(LOG_ERROR);
	InitWindow(SCRW, SCRH, "Cloth Demo");
}

void deinit(VerletCirlce* alloc_circle_mem, Link* alloc_link_mem)
{
	free(alloc_circle_mem);
	free(alloc_link_mem);
	CloseWindow();
}

int main()
{
	Chain chain = create_chain();
	Circles circles = create_circles();

	int grabbed_link_index = -1;
	bool show_circles = false;

	init();
	init_circles(&circles);
	init_chain(&chain, &circles);

	while(!WindowShouldClose())
	{
		for(int i = 0; i < SUB_STEPS; i++)
			update_links(&chain);
		
		update_circles(&circles, &grabbed_link_index);
		pull_cloth(&circles, grabbed_link_index);
		
		if(IsKeyPressed(KEY_C))
			show_circles = !show_circles;

		BeginDrawing();
			ClearBackground(BLACK);
			if(show_circles) draw_circles(&circles);
			draw_links(&chain);
			DrawFPS(0, 0);
		EndDrawing();
	}

	return 0;    
}