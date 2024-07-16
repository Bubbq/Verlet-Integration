#include "headers/raylib.h"
#include "headers/raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    Vector2 curr_pos;
    Vector2 old_pos;
    Vector2 acceleration;
    float radius;
    Color color;
} VerletCirlce;

typedef struct
{
    int size;
    size_t capacity;
    VerletCirlce* circle;
} Circles;

typedef struct
{
    int size;
    int* indicies;
    size_t capacity;
} IndexList;

typedef struct 
{
    Vector2 start;
    IndexList IndexList;
} Cell;

typedef Cell Grid;

typedef struct
{
   double startTime;
   double lifeTime;
} Timer;

const int FPS = 60;
const int STEPS = 8;

const int SCRH = 900;
const int SCRW = 900;
const Vector2 CENTER = { SCRW / 2.0f, SCRH / 2.0f };

const float GRAVITY = 1000.0f;
float grav_strength = GRAVITY;
float BALLS_PER_S = 10;

const float MINR = 100.0f;
const float MAXR = 400.0f;
float RADIUS = 300.0f;

const int CSIZE = 20;
const int COL = ((MAXR * 2) / CSIZE), ROW = ((MAXR * 2) / CSIZE);

void start_timer(Timer *timer, double lifetime)
{
   timer->startTime = GetTime();
   timer->lifeTime = lifetime;
}

bool timer_done(Timer timer)
{
   return GetTime() - timer.startTime >= timer.lifeTime;
}

Color get_random_color()
{
    switch (GetRandomValue(0, 23))
    {
        case 0: return LIGHTGRAY;
        case 1: return GRAY;
        case 2: return DARKGRAY;
        case 3: return YELLOW;
        case 4: return GOLD;
        case 5: return ORANGE;
        case 6: return PINK;
        case 7: return RED;
        case 8: return MAROON;
        case 9: return GREEN;
        case 10: return LIME;
        case 11: return DARKGREEN;
        case 12: return SKYBLUE;
        case 13: return BLUE;
        case 14: return DARKBLUE;
        case 15: return PURPLE;
        case 16: return VIOLET;
        case 17: return DARKPURPLE;
        case 18: return BEIGE;
        case 19: return BROWN;
        case 20: return DARKBROWN;
        case 21: return WHITE;
        case 22: return MAGENTA;
        case 23: return RAYWHITE;
        default: return WHITE;
    }
}

void resize_index_list(IndexList* il)
{
    il->capacity *= 2;
    il->indicies = realloc(il->indicies, il->capacity);
}

void resize_circles(Circles* c)
{
    c->capacity *= 2;
    c->circle = realloc(c->circle, c->capacity);
}

void add_circle_index(IndexList* il, int index)
{
    if((il->size * sizeof(int)) == il->capacity) resize_index_list(il);
    il->indicies[il->size++] = index;
}

void add_verlet_circle(Circles* circles, VerletCirlce vc)
{
    if((circles->size * sizeof(VerletCirlce)) == circles->capacity) resize_circles(circles);
    circles->circle[circles->size++] = vc;
}

void add_balls(Timer* timer, Circles* circles, float radius)
{
    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && timer_done(*timer) && (CheckCollisionPointCircle(GetMousePosition(), CENTER, RADIUS)) && (Vector2Distance(GetMousePosition(), CENTER) < (RADIUS - radius)))
    {
        start_timer(timer, (1.0f / BALLS_PER_S));
        Vector2 direction = Vector2Normalize(Vector2Subtract(GetMousePosition(), CENTER));
        add_verlet_circle(circles, (VerletCirlce){GetMousePosition(), GetMousePosition(), Vector2Scale(direction, (GRAVITY * 10.0f * -1.0f)), radius, get_random_color()});
    }
}

void delete_verlet_circle(Circles* circles, int pos)
{
    for(int i = pos; i < circles->size; i++) circles->circle[i] = circles->circle[i + 1];
    circles->size--;
}

void remove_balls(Circles* circles)
{
    for(int i = 0; i < circles->size; i++)
    {
        if(CheckCollisionCircles(GetMousePosition(), 5, circles->circle[i].curr_pos, circles->circle[i].radius))
        {
            delete_verlet_circle(circles, i);
        }
    }
}

void change_game_settings(int ball_count, float* radius, float* add_speed, float* ball_radius, float* gs)
{
    char text[1024];

	sprintf(text, "%.0f BALL%s PER SECOND", *add_speed, (*add_speed > 1) ? "S" : "");
	GuiSliderBar((Rectangle){MeasureText("ADD SPEED", 10) + 10, 5, 80, 10}, "ADD SPEED", text, add_speed, 1, 40);

	sprintf(text, "%.0fpx", (*radius));
	GuiSliderBar((Rectangle){MeasureText("BORDER RADIUS", 10) + 10, 23, 80, 10}, "BORDER RADIUS", text, radius, MINR, MAXR);
    
    sprintf(text, "%.0fpx", (*ball_radius)); 
	GuiSliderBar((Rectangle){MeasureText("BALL RADIUS", 10) + 10, 41, 80, 10}, "BALL RADIUS", text, ball_radius, 5, 10);

    sprintf(text, "%.0f", (*gs)); 
	GuiSliderBar((Rectangle){MeasureText("GRAVITY STRENGTH", 10) + 10, 59, 80, 10}, "GRAVITY STRENGTH", "", gs, 0, (GRAVITY * 5));

	sprintf(text, "BALL COUNT: %d", ball_count);
    DrawText(text, 5, 79, 10, GRAY);

    DrawFPS(SCRW - 75, 0);
}

void draw_cells(Grid grid[ROW][COL])
{
    for(int r = 0; r < ROW; r++) 
        for(int c = 0; c < COL; c++)  
            DrawRectangleLines(grid[r][c].start.x, grid[r][c].start.y, CSIZE, CSIZE, (c % 2) == 0 ? RED : GREEN);
}

void draw_circles(Circles* circles)
{
    int i = 0;
    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i)) DrawCircleV(vc->curr_pos, vc->radius, vc->color);
}

// O(N^2) sol
// void handle_circle_collision(Circles* all_cicles, VerletCirlce* vc)
// {
//     for(int i = 0; i < all_cicles->size; i++)
//     {
//         if(Vector2Equals(all_cicles->circle[i].curr_pos, vc->curr_pos)) continue;

//         if(CheckCollisionCircles(vc->curr_pos, vc->radius, all_cicles->circle[i].curr_pos, all_cicles->circle[i].radius))
//         {
//             float distance = Vector2Distance(vc->curr_pos, all_cicles->circle[i].curr_pos);
//             // how much the 2 circles need to move by 
//             float delta = (vc->radius + all_cicles->circle[i].radius) - distance;
//             // the axis of collision
//             Vector2 n = Vector2Normalize(Vector2Subtract(vc->curr_pos, all_cicles->circle[i].curr_pos));
//             // move circles in opposite directions so that now they have appropriate distance 
//             vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Scale(n, (delta * 0.5f)));
//             all_cicles->circle[i].curr_pos = Vector2Subtract(all_cicles->circle[i].curr_pos, Vector2Scale(n, (delta * 0.5f)));
//         }
//     }
// }

void handle_circle_collision(VerletCirlce* vc1, VerletCirlce* vc2)
{
    if(CheckCollisionCircles(vc1->curr_pos, vc1->radius, vc2->curr_pos, vc2->radius))
    {
        float distance = Vector2Distance(vc1->curr_pos, vc2->curr_pos);
        
        // how much the 2 circles need to move by 
        float delta = (vc1->radius + vc2->radius) - distance;
        
        // the axis of collision
        Vector2 n = Vector2Normalize(Vector2Subtract(vc1->curr_pos, vc2->curr_pos));
        
        // move circles away from one another 
        vc1->curr_pos = Vector2Add(vc1->curr_pos, Vector2Scale(n, (delta * 0.5f)));
        vc2->curr_pos = Vector2Subtract(vc2->curr_pos, Vector2Scale(n, (delta * 0.5f)));
    }
}

void grid_circle_collision(Grid grid[ROW][COL], Circles* circles)
{
    for (int r = 0; r < ROW; r++)
    {
        for (int c = 0; c < COL; c++)
        {
            // same cell circle coll.
            IndexList* il = &grid[r][c].IndexList;

            for (int i = 0; i < il->size; i++) 
                for (int j = i + 1; j < il->size; j++) 
                    handle_circle_collision(&circles->circle[il->indicies[i]], &circles->circle[il->indicies[j]]);

            // neighbor cell circle collisions
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    int nr = r + dx;
                    int nc = c + dy;
                
                    // out of bounds case
                    if ((nr < 0 || nr >= ROW) || (nc < 0 || nc >= ROW) || (nr == r && nc == c)) continue;
                    
                    for (int i = 0; i < il->size; i++)
                        for (int j = 0; j < grid[nr][nc].IndexList.size; j++)
                            handle_circle_collision(&circles->circle[grid[r][c].IndexList.indicies[i]], &circles->circle[grid[nr][nc].IndexList.indicies[j]]);
                }
            }
        }
    }
}

void handle_border_collision(Vector2* curr_pos, float radius)
{
    Vector2 n = Vector2Normalize(Vector2Subtract(*curr_pos, CENTER));
    *curr_pos = Vector2Add(CENTER, Vector2Scale(n,(RADIUS - radius)));
}

void add_circle_to_cell(Grid grid[ROW][COL], int c_index, Vector2 position)
{
    int c = ((position.x - grid[0][0].start.x) / CSIZE);
    int r = ((position.y - grid[0][0].start.y) / CSIZE);
    
    if((r < ROW) && (c < COL)) add_circle_index(&grid[r][c].IndexList, c_index);
}

void apply_gravity(Vector2* acceleration)
{
    // acheive an acceleration of <0, grav_strength>, like earth, in a second
    Vector2 delta = Vector2Scale((Vector2){(0 - acceleration->x), (grav_strength - acceleration->y)}, GetFrameTime());

    // decceleration
    *acceleration = Vector2Add(*acceleration, delta);
}

void update_position(VerletCirlce* vc, float dt)
{
    Vector2 velocity = Vector2Subtract(vc->curr_pos, vc->old_pos);
    velocity = Vector2Subtract(velocity, Vector2Scale(velocity, dt));
    vc->old_pos = vc->curr_pos;

    // verlet integration formula -> v(n+1) = v(n) + v + adt^2
    vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Add(velocity, Vector2Scale(vc->acceleration, powf(dt, 2.0f))));
}

void clear_grid_index_lists(Grid grid[ROW][COL])
{
    for(int r = 0; r < ROW; r++) 
        for(int c = 0; c < COL; c++) 
            grid[r][c].IndexList.size = 0;
}

void update_circles(Circles* c, Grid grid[ROW][COL], float dt)
{
    clear_grid_index_lists(grid);

    for(int i = 0; i < c->size; i++)
    {
        VerletCirlce* vc = (c->circle + i);
        
        update_position(vc, dt);
        apply_gravity(&vc->acceleration);
        add_circle_to_cell(grid, i, vc->curr_pos);
        // handle_circle_collision(c, vc);
        if((Vector2Distance(vc->curr_pos, CENTER) + vc->radius) >= RADIUS) handle_border_collision(&vc->curr_pos, vc->radius);

    }
    grid_circle_collision(grid, c);
}

void init_grid(Grid grid[ROW][COL])
{
    Vector2 current_position = {(CENTER.x - MAXR), (CENTER.y - MAXR)};

    for(int r = 0; r < ROW; r++, current_position = (Vector2){(CENTER.x - MAXR), (current_position.y + CSIZE)})
    {
        for(int c = 0; c < COL; c++, current_position.x += CSIZE)
        {
            grid[r][c] = (Cell){ current_position, (IndexList){ 0, malloc(sizeof(int)), sizeof(int) }};
        }
    }
}

void init(Grid grid[ROW][COL])
{
    init_grid(grid);

    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Verlet Circle Playground");
}

int main()
{
    float dt;
    float br = 5;
    Timer timer;

    Grid grid[ROW][COL];
    Circles circles = {0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce))};

    init(grid);
    
    while(!WindowShouldClose())
    {
        dt = (GetFrameTime() / STEPS);
        add_balls(&timer, &circles, br);
        for(int i = 0; i < STEPS; i++) update_circles(&circles, grid, dt);   
        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) remove_balls(&circles);

        BeginDrawing();
            ClearBackground(BLACK);
            change_game_settings(circles.size, &RADIUS, &BALLS_PER_S, &br, &grav_strength);
            DrawCircleLinesV(CENTER, RADIUS, RAYWHITE);
            draw_circles(&circles);
            // draw_cells(grid);
        EndDrawing();
    }
    
    // deinit
    for(int r = 0; r < ROW; r++) for(int c = 0; c < COL; c++) free(grid[r][c].IndexList.indicies);
    free(circles.circle);
    CloseWindow();

    return 0;    
}

// to do: make restrictions on changing radius of outer circle wen its filled
// make editor struct for characteristics in the playground