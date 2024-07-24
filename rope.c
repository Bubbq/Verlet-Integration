#include "headers/raylib.h"
#include "headers/raymath.h"
#include <math.h>
#include <stddef.h>

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#include <stdlib.h>
#include <stdio.h>

const int R = 10;
const int CC = 15;
const int FPS = 60;
const int SUB_STEPS = 8;
const int SCRW = 700, SCRH = 700;

const Vector2 GRAVITY = { 0, 750.0f };
const Vector2 CENTER = { (SCRW / 2.0f), (SCRH / 2.0f) };

const float DAMP = 0.950f;

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
    VerletCirlce* vc1;
    VerletCirlce* vc2;
    float target_distance;
} Link;

typedef struct
{
    int size;
    Link* link;
    size_t mem_capacity;
} Chain;

void resize_circles(Circles* c)
{
    c->capacity *= 2;
    c->circle = realloc(c->circle, c->capacity);
}

void resize_Chain(Chain* chain)
{
    chain->mem_capacity *= 2;
    chain->link = realloc(chain->link, chain->mem_capacity);
}

void add_verlet_circle(Circles* circles, VerletCirlce vc)
{
    if((circles->size * sizeof(VerletCirlce)) == circles->capacity) 
        resize_circles(circles);

    circles->circle[circles->size++] = vc;
}

void add_link(Chain* chain, Link link)
{
    if((chain->size * sizeof(link)) == chain->mem_capacity) 
        resize_Chain(chain);

    chain->link[chain->size++] = link;
}

void draw_circles(Circles* circles)
{
    int i = 0;
    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i)) DrawCircleSector(vc->curr_pos, vc->radius, 0, 360, 1, vc->color);
}

void init()
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Verlet Chain");
}

void deinit(VerletCirlce* alloc_circle_mem, Link* alloc_link_mem)
{
    free(alloc_circle_mem);
    free(alloc_link_mem);
    CloseWindow();
}

void select_circle_to_move(Circles* circles, int* current_ball)
{
    for(int i = 0; i < circles->size; i++)
        if((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (CheckCollisionPointCircle(GetMousePosition(), circles->circle[i].curr_pos, circles->circle[i].radius))))
            *current_ball = i;
}

int main()
{
    int current_ball;
    Circles circles = {0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce))};
    Chain chain = { 0, malloc(sizeof(Link)), sizeof(Link) };
    
    // adding circles   
    for(int i = 0; i < CC; i++)
        add_verlet_circle(&circles, (VerletCirlce){(Vector2){ CENTER.x, CENTER.y + (i * R * 2)}, (Vector2){CENTER.x, CENTER.y + (i * R * 2)}, GRAVITY, R, RAYWHITE });

    // applying Chain
    for(int i = 0, j = 1; j < circles.size; i++, j++)
        add_link(&chain, (Link){ (circles.circle + i), (circles.circle + j), (R * 2) });

    init();
    
    while(!WindowShouldClose())
    {
        float dt = (GetFrameTime() / SUB_STEPS);

        select_circle_to_move(&circles, &current_ball);

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) circles.circle[current_ball].curr_pos = GetMousePosition();

        for(int i = 0; i < circles.size; i++)
        {
            // circle collisions
            for(int j = 0; j < circles.size; j++)
            {
                if(i == j) continue;

                if(Vector2Distance(circles.circle[i].curr_pos, circles.circle[j].curr_pos) <= (circles.circle[i].radius + circles.circle[j].radius))
                {
                    float delta = (circles.circle[i].radius + circles.circle[j].radius) - Vector2Distance(circles.circle[i].curr_pos, circles.circle[j].curr_pos);
                    Vector2 n = Vector2Normalize(Vector2Subtract(circles.circle[i].curr_pos, circles.circle[j].curr_pos));

                    circles.circle[i].curr_pos = Vector2Add(circles.circle[i].curr_pos, Vector2Scale(n, (delta / 2.0f)));
                    circles.circle[j].curr_pos = Vector2Subtract(circles.circle[j].curr_pos, Vector2Scale(n, (delta / 2.0f)));
                }
            }

            // gravity
            circles.circle[i].curr_pos = Vector2Add(circles.circle[i].curr_pos, Vector2Scale(GRAVITY, dt));
            
            // border collsion
            if((circles.circle[i].curr_pos.y + circles.circle[i].radius) >= SCRH)
                circles.circle[i].curr_pos.y = (SCRH - circles.circle[i].radius);

            if((circles.circle[i].curr_pos.y + circles.circle[i].radius) <= 0)
                circles.circle[i].curr_pos.y = circles.circle[i].radius;

            if((circles.circle[i].curr_pos.x + circles.circle[i].radius) >= SCRW)
                circles.circle[i].curr_pos.x = (SCRW - circles.circle[i].radius);
            
            if((circles.circle[i].curr_pos.x + circles.circle[i].radius) <= 0)
                circles.circle[i].curr_pos.x = circles.circle[i].radius;

            // updating positions
            Vector2 velocity = Vector2Scale(Vector2Subtract(circles.circle[i].curr_pos, circles.circle[i].old_pos), DAMP);
            circles.circle[i].old_pos = circles.circle[i].curr_pos;
            circles.circle[i].curr_pos = Vector2Add(circles.circle[i].curr_pos, Vector2Add(velocity, Vector2Scale(circles.circle[i].acceleration, (powf(dt, 2.0f)))));
        }

        for(int i = 0; i < chain.size; i++)
            if((Vector2Distance(chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos)) >= chain.link[i].target_distance)
            {
                float circle_distance = Vector2Distance(chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos);
                float delta = chain.link[i].target_distance - circle_distance;
                Vector2 n = Vector2Normalize(Vector2Subtract(chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos));

                chain.link[i].vc1->curr_pos = Vector2Add(chain.link[i].vc1->curr_pos, Vector2Scale(n, (delta / 2.0f)));
                chain.link[i].vc2->curr_pos = Vector2Subtract(chain.link[i].vc2->curr_pos, Vector2Scale(n, (delta / 2.0f)));
            }

        BeginDrawing();
            ClearBackground(BLACK);
            draw_circles(&circles);
            DrawFPS(0, 0);
        EndDrawing();
    }
    
    deinit(circles.circle, chain.link);
    return 0;    
}