#include "headers/timer.h"
#include "headers/raylib.h"
#include "headers/raymath.h"
#include "headers/circle.h"
#include "headers/physics.h"
#include "headers/spatial_partition.h"

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#include <stdlib.h>
#include <stdio.h>

const int FPS = 60;
const int SUB_STEPS = 8;

const int SCRH = 900;
const int SCRW = 900;
const Vector2 CENTER = { SCRW / 2.0f, SCRH / 2.0f };

const float GRAVITY = 1000.0f;

const float MINR = 100.0f;
const float MAXR = BORDER_RADIUS;

typedef struct
{
	float constraint_radius;
	float ball_radius;
	float balls_per_second;
	float gravity_strength;
} PlaygroundEditor;

PlaygroundEditor create_editor()
{
	PlaygroundEditor playground_editor;

	playground_editor.constraint_radius = 300;
	playground_editor.ball_radius = 5;
	playground_editor.balls_per_second = 10;
	playground_editor.gravity_strength = GRAVITY;

	return playground_editor;
}

Color get_random_color()
{
	switch (GetRandomValue(0, 2))
	{
		case 0: return RED;
		case 1: return BLUE;
		case 2: return YELLOW;
		default: return BLANK;
	}
}

void add_balls(Timer* timer, Circles* circles, PlaygroundEditor pe)
{
	const int INIT_ACCEL = 20;

	if((IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && (timer_done(*timer)) && (CheckCollisionPointCircle(GetMousePosition(), CENTER, pe.constraint_radius)) && (Vector2Distance(GetMousePosition(), CENTER) < (pe.constraint_radius - pe.ball_radius)))
	{
		start_timer(timer, (1 / pe.balls_per_second));

		VerletCirlce projectile;
		projectile.color = get_random_color();
		projectile.radius = pe.ball_radius;
		projectile.status = FREE;
		projectile.acceleration = Vector2Scale(Vector2Normalize(Vector2Subtract(GetMousePosition(), CENTER)), (GRAVITY * INIT_ACCEL * -1));
		projectile.previous_position = projectile.current_position = GetMousePosition();

		add_verlet_circle(circles,projectile);
	}
}

void handle_ball_overflow(Circles* circles, int ball_capacity)
{
	while((circles->size > ball_capacity))
		circles->size--;            
}

void remove_balls(Circles* circles)
{
	const int ERASER_SIZE = 10;

	for(int i = 0; i < circles->size; i++)
	{
		if(CheckCollisionCircles(GetMousePosition(), ERASER_SIZE, circles->circle[i].current_position, circles->circle[i].radius))
			delete_verlet_circle(circles, i);
	}
}

float max_circle_count(float R, float r)
{
	return (0.83 * (powf(R, 2) / powf(r, 2)) - 1.9);
}

float average_radius(Circles* circles)
{
	float average_r = 0;

	for(int i = 0; i < circles->size; i++) 
		average_r += circles->circle[i].radius;

	return (circles->size > 0) ? (average_r / circles->size) : 5;
}

void change_playground_statistics(PlaygroundEditor* statistics, int ball_count)
{
	char text[100];

	sprintf(text, "%.0f BALL(S) PER SECOND", statistics->balls_per_second);
	GuiSliderBar((Rectangle){MeasureText("ADD SPEED", 10) + 10, 5, 80, 10}, "ADD SPEED", text, &statistics->balls_per_second, 1, 25);

	sprintf(text, "%.0fpx", statistics->constraint_radius);
	GuiSliderBar((Rectangle){MeasureText("BORDER RADIUS", 10) + 10, 23, 80, 10}, "BORDER RADIUS", text, &statistics->constraint_radius, MINR, MAXR);
	
	sprintf(text, "%.0fpx", statistics->ball_radius); 
	GuiSliderBar((Rectangle){MeasureText("BALL RADIUS", 10) + 10, 41, 80, 10}, "BALL RADIUS", text, &statistics->ball_radius, 5, 10);

	sprintf(text, "%.0f", statistics->gravity_strength); 
	GuiSliderBar((Rectangle){MeasureText("GRAVITY STRENGTH", 10) + 10, 59, 80, 10}, "GRAVITY STRENGTH", "", &statistics->gravity_strength, 0, (GRAVITY * 3));

	sprintf(text, "BALL COUNT: %d", ball_count);
	DrawText(text, 5, 79, 10, GRAY);
}

void update_circles(Circles* circles, Grid grid[ROW][COL], PlaygroundEditor statistics, float dt)
{
	clear_grid_index_lists(grid);

	int i = 0;
	for(VerletCirlce* vc = (circles->circle + i); (i < circles->size); i++, vc = (circles->circle + i))
	{
		add_circle_to_grid(grid, i, vc->current_position);
		update_position(vc, 0.995, dt);
		apply_gravity(vc, (Vector2){ 0, statistics.gravity_strength }, GetFrameTime());
		handle_border_collision(vc, CENTER, (Vector2){ 0, statistics.gravity_strength }, statistics.constraint_radius);
	}

	grid_circle_collision(grid, circles);
}

void init()
{
	SetTargetFPS(FPS);
	SetTraceLogLevel(LOG_ERROR);
	InitWindow(SCRW, SCRH, "Verlet Circle Playground");
}

void deinit(VerletCirlce* alloc_circle_mem, Grid grid[ROW][COL])
{
	free(alloc_circle_mem);
	dealloc_grid(grid);
	CloseWindow();
}

int main()
{
	Circles circles = create_circles();
	Grid grid[ROW][COL];
	Timer add_ball_timer;

	PlaygroundEditor settings = create_editor();

	init();
	create_grid(grid, CENTER);
	
	while(!WindowShouldClose())
	{
		float dt = GetFrameTime() / SUB_STEPS;
		float mcc = max_circle_count(settings.constraint_radius, average_radius(&circles));

		add_balls(&add_ball_timer, &circles, settings);
		handle_ball_overflow(&circles, mcc);
		
		if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) 
			remove_balls(&circles);

		for(int i = 0; i < SUB_STEPS; i++) 
			update_circles(&circles, grid, settings, dt);   
		
		BeginDrawing();
			ClearBackground(BLACK);
			draw_circles(&circles);
			DrawFPS(SCRW - 75, 0);
			change_playground_statistics(&settings, circles.size);
			DrawCircleLinesV(CENTER, settings.constraint_radius, RAYWHITE);
		EndDrawing();
	}
	
	deinit(circles.circle, grid);
	return 0;    
}