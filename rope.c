#include "headers/raylib.h"
#include "headers/physics.h"

#include <stdlib.h>
#include <stdio.h>

const int R = 10;
const int CC = 15;
const int FPS = 60;
const int SUB_STEPS = 8;
const int SCRW = 700, SCRH = 700;

const Vector2 GRAVITY = { 0, 2000.0f };
const Vector2 CENTER = { (SCRW / 2.0f), (SCRH / 2.0f) };

const float DAMP = 0.950f;

void draw_circles(Circles* circles)
{
    int i = 0;
    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i)) DrawCircleSector(vc->current_position, vc->radius, 0, 360, 1, vc->color);
}

void init_rope(Circles* circles)
{
    for(int i = 0; i < CC; i++)
    add_verlet_circle(circles, (VerletCirlce){ RAYWHITE, R, FREE, GRAVITY, (Vector2){ CENTER.x, CENTER.y + (i * R * 2)}, (Vector2){ CENTER.x, CENTER.y + (i * R * 2)} });
}

void init_chain(Chain* chain, Circles* circles)
{
    for(int i = 0, j = 1; j < circles->size; i++, j++)
        add_link(chain, (Link){ (circles->circle + i), (circles->circle + j), (R * 2) });
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
        if((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (CheckCollisionPointCircle(GetMousePosition(), circles->circle[i].current_position, circles->circle[i].radius))))
            *current_ball = i;
}

void update_circles(Circles* circles)
{
    for(int i = 0; i < circles->size; i++)
    {
        // circle collisions
        for(int j = 0; j < circles->size; j++)
        {
            if(i == j) continue;
            handle_verlet_circle_collision(&circles->circle[i], &circles->circle[j]);
        }

        apply_gravity((circles->circle + i), GRAVITY, GetFrameTime());
        
        // border collsion
        if((circles->circle[i].current_position.y + circles->circle[i].radius) >= SCRH)
            circles->circle[i].current_position.y = (SCRH - circles->circle[i].radius);

        if((circles->circle[i].current_position.y + circles->circle[i].radius) <= 0)
            circles->circle[i].current_position.y = circles->circle[i].radius;

        if((circles->circle[i].current_position.x + circles->circle[i].radius) >= SCRW)
            circles->circle[i].current_position.x = (SCRW - circles->circle[i].radius);
        
        if((circles->circle[i].current_position.x + circles->circle[i].radius) <= 0)
            circles->circle[i].current_position.x = circles->circle[i].radius;

        update_position((circles->circle + i), DAMP, GetFrameTime());
    }
}

void update_links(Chain* chain)
{
    for(int i = 0; i < chain->size; i++)
        maintain_link((chain->link + i));
}

int main()
{
    int current_ball;
    Circles circles = {0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce))};
    Chain chain = { 0, malloc(sizeof(Link)), sizeof(Link) };

    init();
    init_rope(&circles);
    init_chain(&chain, &circles);
    
    while(!WindowShouldClose())
    {
        select_circle_to_move(&circles, &current_ball);

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
            circles.circle[current_ball].current_position = GetMousePosition();

        update_circles(&circles);
        update_links(&chain);

        BeginDrawing();
            ClearBackground(BLACK);
            draw_circles(&circles);
            DrawFPS(0, 0);
        EndDrawing();
    }
    
    deinit(circles.circle, chain.link);
    return 0;    
}