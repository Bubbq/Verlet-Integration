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
    IndexList index_list;
} Cell;

typedef Cell Grid;

typedef struct
{
   double startTime;
   double lifeTime;
} Timer;

typedef struct
{
    float constraint_radius;
    float ball_radius;
    float balls_per_second;
    float gravity_strength;
} PlaygroundEditor;

const int FPS = 60;
const int SUB_STEPS = 8;

const int SCRH = 900;
const int SCRW = 900;
const Vector2 CENTER = { SCRW / 2.0f, SCRH / 2.0f };

const float GRAVITY = 1000.0f;

const float MINR = 100.0f;
const float MAXR = 400.0f;

// for optimal preformance, let the size of a cell be the diameter of the balls you make
const int CSIZE = 20;
const int ROW = ((MAXR * 2) / CSIZE), COL = ((MAXR * 2) / CSIZE);

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
    switch (GetRandomValue(0, 2))
    {
        case 0: return RED;
        case 1: return BLUE;
        case 2: return YELLOW;
        default: return BLANK;
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
    if((il->size * sizeof(int)) == il->capacity) 
        resize_index_list(il);
    
    il->indicies[il->size++] = index;
}

void add_verlet_circle(Circles* circles, VerletCirlce vc)
{
    if((circles->size * sizeof(VerletCirlce)) == circles->capacity) 
        resize_circles(circles);

    circles->circle[circles->size++] = vc;
}

void add_balls(Timer* timer, Circles* circles, float balls_per_second, float constraint_radius, float ball_radius)
{
    if((IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && (timer_done(*timer)) && (CheckCollisionPointCircle(GetMousePosition(), CENTER, constraint_radius)) && (Vector2Distance(GetMousePosition(), CENTER) < (constraint_radius - ball_radius)))
    {
        start_timer(timer, (1.0f / balls_per_second));
        Vector2 direction = Vector2Normalize(Vector2Subtract(GetMousePosition(), CENTER));
        add_verlet_circle(circles, (VerletCirlce){GetMousePosition(), GetMousePosition(), Vector2Scale(direction, (-GRAVITY * 30)), ball_radius, get_random_color()});
    }
}

void delete_verlet_circle(Circles* circles, int pos)
{
    for(int i = pos; i < circles->size; i++) 
        circles->circle[i] = circles->circle[i + 1];
    
    circles->size--;
}

void remove_balls(Circles* circles)
{
    for(int i = 0; i < circles->size; i++)
    {
        if(CheckCollisionCircles(GetMousePosition(), 10, circles->circle[i].curr_pos, circles->circle[i].radius))
        {
            delete_verlet_circle(circles, i);
        }
    }
}

// how many circles of radius r can fit in a circle of radius R
float max_circle_count(float R, float r)
{
    return (0.83 * (powf(R, 2) / powf(r, 2)) - 1.9);
}

float average_radius(Circles* circles)
{
    float average_r = 0;

    for(int i = 0; i < circles->size; i++) 
        average_r += circles->circle[i].radius;

    return (circles->size > 0) ? (average_r / circles->size) : 0;
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
	GuiSliderBar((Rectangle){MeasureText("GRAVITY STRENGTH", 10) + 10, 59, 80, 10}, "GRAVITY STRENGTH", "", &statistics->gravity_strength, 0, (GRAVITY * 2));

	sprintf(text, "BALL COUNT: %d", ball_count);
    DrawText(text, 5, 79, 10, GRAY);

    DrawFPS(SCRW - 75, 0);
}

void draw_circles(Circles* circles)
{
    int i = 0;
    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i)) DrawCircleSector(vc->curr_pos, vc->radius, 0, 360, 1, vc->color);
}

void handle_circle_collision(VerletCirlce* vc1, VerletCirlce* vc2)
{
    const float SCALE = 0.875f;

    if(CheckCollisionCircles(vc1->curr_pos, vc1->radius, vc2->curr_pos, vc2->radius))
    {
        // how much the 2 circles need to move by 
        float delta = (vc1->radius + vc2->radius) - Vector2Distance(vc1->curr_pos, vc2->curr_pos);
        
        // the axis of collision
        Vector2 n = Vector2Normalize(Vector2Subtract(vc1->curr_pos, vc2->curr_pos));
        
        // move circles away from one another 
        vc1->curr_pos = Vector2Add(vc1->curr_pos, Vector2Scale(n, (delta / 2) * SCALE));
        vc2->curr_pos = Vector2Subtract(vc2->curr_pos, Vector2Scale(n, (delta / 2) * SCALE));
    }
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
                    handle_circle_collision(&circles->circle[il->indicies[i]], &circles->circle[il->indicies[j]]);

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
                            handle_circle_collision(&circles->circle[il->indicies[i]], &circles->circle[nil->indicies[j]]);
                }
        }
}

void handle_border_collision(Vector2* curr_pos, Vector2* accel, float constraint_radius, float ball_radius, float gravity_strength)
{
    if((Vector2Distance(*curr_pos, CENTER) + ball_radius) >= constraint_radius)
    {
        Vector2 n = Vector2Normalize(Vector2Subtract(*curr_pos, CENTER));
        *curr_pos = Vector2Add(CENTER, Vector2Scale(n,(constraint_radius - ball_radius)));
        *accel = (Vector2){ 0, gravity_strength };
    }
}

void add_circle_to_cell(Grid grid[ROW][COL], int c_index, Vector2 position)
{
    int c = ((position.x - grid[0][0].start.x) / CSIZE);
    int r = ((position.y - grid[0][0].start.y) / CSIZE);
    
    if((r < ROW) && (c < COL)) add_circle_index(&grid[r][c].index_list, c_index);
}

// deccelerate current accel to (0, gravity_strength) in one second
void apply_gravity(Vector2* acceleration, float gravity_strength)
{
    *acceleration = Vector2Add(*acceleration, Vector2Scale((Vector2){ (0 - acceleration->x), (gravity_strength - acceleration->y) }, GetFrameTime()));
}

void update_position(VerletCirlce* vc, float dt)
{
    // slowdown factor
    const float DAMP = 0.995f;
    const float MAX_V = 10.0f;
    
    Vector2 velocity = Vector2Scale(Vector2Subtract(vc->curr_pos, vc->old_pos), DAMP);
    if((Vector2Length(velocity) > MAX_V)) velocity = (Vector2){};
    
    vc->old_pos = vc->curr_pos;

    // verlet integration formula -> v(n+1) = v(n) + v + adt^2
    vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Add(velocity, Vector2Scale(vc->acceleration, powf(dt, 2.0f))));
}

void clear_grid_index_lists(Grid grid[ROW][COL])
{
    for(int r = 0; r < ROW; r++) 
        for(int c = 0; c < COL; c++) 
            grid[r][c].index_list.size = 0;
}

void update_circles(Circles* circles, Grid grid[ROW][COL], PlaygroundEditor statistics, float dt)
{
    clear_grid_index_lists(grid);

    int i = 0;
    for(VerletCirlce* vc = (circles->circle + i); (i < circles->size); i++, vc = (circles->circle + i))
    {
        add_circle_to_cell(grid, i, vc->curr_pos);
        update_position(vc, dt);
        apply_gravity(&vc->acceleration, statistics.gravity_strength);
        handle_border_collision(&vc->curr_pos, &vc->acceleration, statistics.constraint_radius, vc->radius, statistics.gravity_strength);
    }

    grid_circle_collision(grid, circles);
}

void init_grid(Grid grid[ROW][COL])
{
    Vector2 current_position = {(CENTER.x - MAXR), (CENTER.y - MAXR)};

    for(int r = 0; r < ROW; r++, current_position = (Vector2){(CENTER.x - MAXR), (current_position.y + CSIZE)})
        for(int c = 0; c < COL; c++, current_position.x += CSIZE)
            grid[r][c] = (Cell){ current_position, (IndexList){ 0, malloc(sizeof(int)), sizeof(int) }};
}

void init(Grid grid[ROW][COL], PlaygroundEditor* pe, Circles* c)
{
    init_grid(grid);
    *pe = (PlaygroundEditor){ 300, 5, 10, GRAVITY };
    *c = (Circles){ 0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce)) };

    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Verlet Circle Playground");
}

void deinit(VerletCirlce* alloc_circle_mem, Grid grid[ROW][COL])
{
    for(int r = 0; r < ROW; r++) 
        for(int c = 0; c < COL; c++)
            free(grid[r][c].index_list.indicies);
    
    free(alloc_circle_mem);
    CloseWindow();
}

int main()
{
    Circles circles;
    Grid grid[ROW][COL];
    
    PlaygroundEditor pe;
    Timer add_ball_timer;

    init(grid, &pe, &circles);
    
    while(!WindowShouldClose())
    {
        float dt = (GetFrameTime() / SUB_STEPS);
        float mcc = max_circle_count(pe.constraint_radius, average_radius(&circles));

        add_balls(&add_ball_timer, &circles, pe.balls_per_second, pe.constraint_radius, pe.ball_radius);
        
        while((circles.size > mcc))
            circles.size--;            

        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) 
            remove_balls(&circles);

        for(int i = 0; i < SUB_STEPS; i++) 
            update_circles(&circles, grid, pe, dt);   
        
        BeginDrawing();
            ClearBackground(BLACK);
            draw_circles(&circles);
            change_playground_statistics(&pe, circles.size);
            DrawCircleLinesV(CENTER, pe.constraint_radius, RAYWHITE);
        EndDrawing();
    }
    
    deinit(circles.circle, grid);
    return 0;    
}
