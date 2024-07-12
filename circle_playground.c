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
float RADIUS = 300.0f;

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

void resize_circles(Circles* c)
{
    c->capacity *= 2;
    c->circle = realloc(c->circle, c->capacity);
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
        add_verlet_circle(circles, (VerletCirlce){GetMousePosition(), GetMousePosition(), Vector2Scale(direction, (-GRAVITY * 10)), radius, get_random_color()});
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

void display_stats(int ball_count, float* radius, float* add_speed, float* ball_radius, float* gs)
{
    char text[1024];

	sprintf(text, "%.0f BALL%s PER SECOND", *add_speed, (*add_speed > 1) ? "S" : "");
	GuiSliderBar((Rectangle){MeasureText("ADD SPEED", 10) + 10, 5, 80, 10}, "ADD SPEED", text, add_speed, 1, 20);

	sprintf(text, "%.0fpx", (*radius));
	GuiSliderBar((Rectangle){MeasureText("BORDER RADIUS", 10) + 10, 23, 80, 10}, "BORDER RADIUS", text, radius, 100, 400);
    
    sprintf(text, "%.0fpx", (*ball_radius)); 
	GuiSliderBar((Rectangle){MeasureText("BALL RADIUS", 10) + 10, 41, 80, 10}, "BALL RADIUS", text, ball_radius, 1, 20);

    sprintf(text, "%.0f", (*gs)); 
	GuiSliderBar((Rectangle){MeasureText("GRAVITY STRENGTH", 10) + 10, 59, 80, 10}, "GRAVITY STRENGTH", "", gs, 0, (GRAVITY * 5));

	sprintf(text, "BALL COUNT: %d", ball_count);
    DrawText(text, 5, 79, 10, GRAY);

    DrawFPS(SCRW - 75, 0);
}

void draw_circles(Circles* circles)
{
    for(int i = 0; i < circles->size; i++)
    {
        VerletCirlce* vc = (circles->circle + i);
        DrawCircle(vc->curr_pos.x, vc->curr_pos.y, vc->radius, vc->color);
    }
}

void handle_border_collision(Vector2* curr_pos, float radius)
{
    if((Vector2Distance(*curr_pos, CENTER) + radius) >= RADIUS)
    {
        Vector2 n = Vector2Normalize(Vector2Subtract(*curr_pos, CENTER));
        *curr_pos = Vector2Add(CENTER, Vector2Scale(n,(RADIUS - radius)));
    }
}

void handle_circle_collision(Circles* all_cicles, VerletCirlce* vc)
{
    for(int i = 0; i < all_cicles->size; i++)
    {
        if(Vector2Equals(all_cicles->circle[i].curr_pos, vc->curr_pos)) continue;

        if(CheckCollisionCircles(vc->curr_pos, vc->radius, all_cicles->circle[i].curr_pos, all_cicles->circle[i].radius))
        {
            float distance = Vector2Distance(vc->curr_pos, all_cicles->circle[i].curr_pos);
            // how much the 2 circles need to move by 
            float delta = (vc->radius + all_cicles->circle[i].radius) - distance;
            // the axis of collision
            Vector2 n = Vector2Normalize(Vector2Subtract(vc->curr_pos, all_cicles->circle[i].curr_pos));
            // move circles in opposite directions so that now they have appropriate distance 
            vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Scale(n, (delta * 0.5f)));
            all_cicles->circle[i].curr_pos = Vector2Subtract(all_cicles->circle[i].curr_pos, Vector2Scale(n, (delta * 0.5f)));
        }
    }
}

void apply_gravity(Vector2* acceleration)
{
    // acheive an acceleration of <0, grav_strength>, like earth, in a second
    const float dt = GetFrameTime();
    Vector2 delta = { ((0 - acceleration->x) * dt), ((grav_strength - acceleration->y) * dt) };

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

void update_circles(Circles* c, float dt)
{
    for(int i = 0; i < c->size; i++)
    {
        VerletCirlce* vc = (c->circle + i);
        update_position(vc, dt);
        apply_gravity(&vc->acceleration);
        handle_circle_collision(c, vc);
        handle_border_collision(&vc->curr_pos, vc->radius);
    }
}

void init()
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Verlet Circle Playground");
}

void deinit(VerletCirlce* alloc_mem)
{
    free(alloc_mem);
    CloseWindow();
}

int main()
{
    float dt;
    float br = 5;
    Timer timer;
    Circles circles = {0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce))};
    init();
    while(!WindowShouldClose())
    {
        dt = (GetFrameTime() / STEPS);
        for(int i = 0; i < STEPS; i++) update_circles(&circles, dt);   
        add_balls(&timer, &circles, br);
        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) remove_balls(&circles);
        BeginDrawing();
            display_stats(circles.size, &RADIUS, &BALLS_PER_S, &br, &grav_strength);
            DrawCircleLinesV(CENTER, RADIUS, RAYWHITE);
            draw_circles(&circles);
            ClearBackground(BLACK);
        EndDrawing();
    }
    deinit(circles.circle);
    return 0;    
}