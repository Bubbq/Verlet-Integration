#include "headers/raylib.h"
#include "headers/raymath.h"
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
const int SCRH = 1024;
const int SCRW = 1000;
const float ADD_TIME = 0.1f;
const float GRAVITY = 1000.0f;

const float RADIUS = 300.0f;
const Vector2 CENTER = { SCRW / 2.0f, SCRH / 3.0f };

void start_timer(Timer *timer, double lifetime)
{
   timer->startTime = GetTime();
   timer->lifeTime = lifetime;
}

bool timer_done(Timer timer)
{
   return GetTime() - timer.startTime >= timer.lifeTime;
}

float get_angle(Vector2 v1, Vector2 v2)
{
	float angle = (atan2f((v1.y - v2.y), (v1.x - v2.x)) * RAD2DEG);
	if(angle > 360) angle -= 360;
	else if(angle < 0) angle += 360;
	return angle;
}

Color get_random_color()
{
    int rand = GetRandomValue(0, 25);
    switch (rand)
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
        case 22: return BLACK;
        case 23: return BLANK;
        case 24: return MAGENTA;
        case 25: return RAYWHITE;
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

void add_balls(Timer* timer, Circles* circles)
{
    int radius = GetRandomValue(5, 10);
    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && timer_done(*timer) && (CheckCollisionPointCircle(GetMousePosition(), CENTER, RADIUS)) && (Vector2Distance(GetMousePosition(), CENTER) < (RADIUS - radius)))
    {
        start_timer(timer, ADD_TIME);
        Vector2 direction = Vector2Normalize(Vector2Subtract(GetMousePosition(), CENTER));
        add_verlet_circle(circles, (VerletCirlce){GetMousePosition(), GetMousePosition(), Vector2Scale(direction, (-GRAVITY * 20)), radius, get_random_color()});
    }
}

void delete_verlet_circle(Circles* circles, int pos)
{
    for(int i = pos; i < circles->size; circles->circle[i] = circles->circle[i + 1],i++)
        ;
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

void display_stats(int ball_count)
{
    char ball_count_text[1024];
    sprintf(ball_count_text, "ball count: %d", ball_count);
    DrawText(ball_count_text, 0, 0, 20, LIME);
    DrawFPS(0, 19);
}

void draw_circles(Circles* circles)
{
    for(int i = 0; i < circles->size; i++)
    {
        VerletCirlce* vc = (circles->circle + i);
        DrawCircle(vc->curr_pos.x, vc->curr_pos.y, vc->radius, vc->color);
    }
}

void update_position(VerletCirlce* vc, float dt)
{
    Vector2 velocity = Vector2Subtract(vc->curr_pos, vc->old_pos);
    vc->old_pos = vc->curr_pos;

    // verlet integration formula -> v(n+1) = v(n) + v + adt^2
    vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Add(velocity, Vector2Scale(vc->acceleration, powf(dt, 2.0f))));
}

void handle_border_collision(Vector2* curr_pos, float radius)
{
    if((Vector2Distance(*curr_pos, CENTER) + radius) >= RADIUS)
    {
        float angle = get_angle(*curr_pos, CENTER);
        Vector2 normal_vector = {cosf(angle * DEG2RAD), sinf(angle * DEG2RAD)};
        *curr_pos = Vector2Add(CENTER, Vector2Scale(normal_vector,(RADIUS - radius)));
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
            float angle = get_angle(vc->curr_pos, all_cicles->circle[i].curr_pos);
            // the normal vector relative to the center of the other, collided circle
            Vector2 n = { cosf(angle * DEG2RAD), sinf(angle * DEG2RAD) };
            // move circles in opposite directions so that now they have appropriate distance 
            vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Scale(n, (delta * 0.5f)));
            all_cicles->circle[i].curr_pos = Vector2Subtract(all_cicles->circle[i].curr_pos, Vector2Scale(n, (delta * 0.5f)));
        }
    }
}

void apply_gravity(Vector2* acceleration)
{
    float delta = (20 * GRAVITY * GetFrameTime());
    acceleration->y += (acceleration->y > GRAVITY) ? -delta : delta;
    acceleration->x += (acceleration->x > 0) ? -delta : delta;
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
    InitWindow(SCRW, SCRH, "Verlet Circles");
}

void deinit(VerletCirlce* alloc_mem)
{
    free(alloc_mem);
    CloseWindow();
}

int main()
{
    float dt;
    Timer timer;
    Circles circles = {0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce))};
    init();

    while(!WindowShouldClose())
    {
        dt = (GetFrameTime() / STEPS);
        for(int i = 0; i < STEPS; i++) update_circles(&circles, dt);   
        add_balls(&timer, &circles);
        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) remove_balls(&circles);

        BeginDrawing();
            ClearBackground(BLACK);
            draw_circles(&circles);
            display_stats(circles.size);
            DrawCircleLinesV(CENTER, RADIUS, RAYWHITE);
        EndDrawing();
    }

    deinit(circles.circle);
    return 0;    
}