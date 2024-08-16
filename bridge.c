#include "headers/raylib.h"
#include "headers/raymath.h"
#include "headers/physics.h"
#include <stdio.h>
#include <stdlib.h>

const int FPS = 60;
const int SCRW = 900, SCRH = 900;

const Vector2 CENTER = { (SCRW / 2.0), (SCRH / 2.0) };
const Vector2 WORLD_GRAVITY = { 0, 1000 };

typedef struct
{
    Circles circles;
    Chain chain;
} Bridge;

void delete_projectiles(Circles* projectiles)
{
    for(int i = 0; i < projectiles->size; i++)
    {
        if( (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) && (CheckCollisionPointCircle(GetMousePosition(), projectiles->circle[i].current_position, projectiles->circle[i].radius)))
            remove_verlet_circle(projectiles, i);
    }
}

void projectile_collision(VerletCirlce* projectile, Circles* other_projectiles)
{
    for(int i = 0; i < other_projectiles->size; i++)
    {
        if(Vector2Equals(projectile->current_position, other_projectiles->circle[i].current_position))
            continue;

        if(CheckCollisionCircles(projectile->current_position, projectile->radius, other_projectiles->circle[i].current_position, other_projectiles->circle[i].radius))
            handle_verlet_circle_collision(projectile, (other_projectiles->circle + i));
    }
}

void projectile_bridge_collsion(VerletCirlce* projectile, Circles* bridge_circles)
{
    // bridge collision
    for(int i = 0; i < bridge_circles->size; i++)
    {
        if(CheckCollisionCircles(projectile->current_position, projectile->radius, bridge_circles->circle[i].current_position, bridge_circles->circle[i].radius))
        {
            handle_verlet_circle_collision(projectile, (bridge_circles->circle + i));
            projectile->acceleration = WORLD_GRAVITY;
        }
    }
}

void shoot_projectile(Circles* circles)
{
    const int BALL_RADIUS = 15;

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        VerletCirlce projectile;

        projectile.color = RED;
        projectile.radius = BALL_RADIUS;
        projectile.status = FREE;
        projectile.acceleration = Vector2Scale(Vector2Normalize(Vector2Subtract(GetMousePosition(), CENTER)), -2000);
        projectile.previous_position = projectile.current_position = GetMousePosition();

        add_verlet_circle(circles, projectile);
    }
}

void update_circles(Circles* circles)
{
    // slowdown scale factor
    const float DAMP = 0.975f;

    for(int i = 0; i < circles->size; i++)
    {
        VerletCirlce* vc = &circles->circle[i];

        if(vc->status == FREE)
        {
            apply_gravity(vc, WORLD_GRAVITY, GetFrameTime());
            update_position(vc, DAMP, GetFrameTime());
        }

        if(!CheckCollisionCircleRec(vc->current_position, vc->radius, (Rectangle){ 0, 0, SCRW, SCRH }))
            remove_verlet_circle(circles, i);
    }
}

void connect_bridge(Circles* bridge_circles, Chain* bridge_links)
{
    for(int i = 0; i < (bridge_circles->size - 1); i++)
    {
        Link link;

        link.circle1 = &bridge_circles->circle[i];
        link.circle2 = &bridge_circles->circle[i + 1];
        link.target_distance = link.circle1->radius;

        add_link(bridge_links, link); 
    }
}

void init_bridge(Bridge* bridge)
{
    const int XPAD = 100;
    const int BRIDGE_RADIUS = 10;

    Vector2 current_position = { XPAD, (SCRH * 0.50) };
    int nbridge_circles = (SCRW - (XPAD * 2)) / (BRIDGE_RADIUS * 2); 

    for(int i = 0; i < nbridge_circles; i++)
    {
        VerletCirlce bridge_circle;

        bridge_circle.color = RAYWHITE;
        bridge_circle.radius = BRIDGE_RADIUS;
        bridge_circle.status = ((i == 0) || (i == (nbridge_circles - 1))) ? SUSPENDED : FREE;
        bridge_circle.acceleration = WORLD_GRAVITY;
        bridge_circle.previous_position = bridge_circle.current_position = current_position;

        add_verlet_circle(&bridge->circles, bridge_circle);
        current_position.x += (BRIDGE_RADIUS * 2);
    }

    for(int i = 0; i < (bridge->circles.size - 1); i++)
    {
        Link link;

        link.circle1 = &bridge->circles.circle[i];
        link.circle2 = &bridge->circles.circle[i + 1];
        link.target_distance = link.circle1->radius;

        add_link(&bridge->chain, link); 
    }
}

void init()
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Bridge");
}

int main()
{
    Bridge bridge;

    bridge.circles.size = 0;
    bridge.circles.capacity = sizeof(VerletCirlce);
    bridge.circles.circle = malloc(bridge.circles.capacity);

    bridge.chain.size = 0;
    bridge.chain.capacity = sizeof(Link);
    bridge.chain.link = malloc(bridge.chain.capacity);

    Circles projectiles;

    projectiles.size = 0;
    projectiles.capacity = sizeof(VerletCirlce);
    projectiles.circle = malloc(projectiles.capacity);
    
    init();
    init_bridge(&bridge);

    while(!WindowShouldClose())
    {
        for(int l = 0; l < bridge.chain.size; l++)
            maintain_link(&bridge.chain.link[l]);
        for(int p = 0; p < projectiles.size; p++)
        {
            projectile_bridge_collsion((projectiles.circle + p), &bridge.circles);
            projectile_collision((projectiles.circle + p), &projectiles);
        }
        update_circles(&bridge.circles);
        update_circles(&projectiles);
        shoot_projectile(&projectiles);
        delete_projectiles(&projectiles);
        BeginDrawing();
            ClearBackground(BLACK);
            draw_circles(&bridge.circles);
            draw_circles(&projectiles);
            DrawFPS(0, 0);
        EndDrawing();
    }

    free(bridge.circles.circle);
    CloseWindow();
    return 0;
}

// split link and circle functionality from physics
// clean code