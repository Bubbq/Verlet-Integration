#include "headers/raylib.h"
#include "headers/raymath.h"
#include "headers/physics.h"
#include <stdlib.h>

const int FPS = 60;
const int SCRW = 900, SCRH = 900;

const Vector2 CENTER = { (SCRW / 2.0), (SCRH / 2.0) };
const Vector2 WORLD_GRAVITY = { 0, 1000 };

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
    }
}

void connect_bridge(Circles* bridge_circles, Chain* bridge_links)
{
    for(int i = 0; i < bridge_circles->size - 1; i++)
    {
        Link link;

        link.circle1 = &bridge_circles->circle[i];
        link.circle2 = &bridge_circles->circle[i + 1];
        link.target_distance = link.circle1->radius;

        add_link(bridge_links, link); 
    }
}

void init_bridge(Circles* circles, Chain* chain)
{
    const int XPAD = 100;
    const int BRIDGE_RADIUS = 10;

    Vector2 current_position = { XPAD, (SCRH * 0.5) };
    int nbridge_circles = (SCRW - (XPAD * 2)) / (BRIDGE_RADIUS * 2); 

    for(int i = 0; i < nbridge_circles; i++)
    {
        VerletCirlce bridge_circle;

        bridge_circle.color = RAYWHITE;
        bridge_circle.radius = BRIDGE_RADIUS;
        bridge_circle.status = ((i == 0) || (i == (nbridge_circles - 1))) ? SUSPENDED : FREE;
        bridge_circle.acceleration = WORLD_GRAVITY;
        bridge_circle.previous_position = bridge_circle.current_position = current_position;

        add_verlet_circle(circles, bridge_circle);
        current_position.x += (BRIDGE_RADIUS * 2);
    }

    connect_bridge(circles, chain);
}

void init()
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Bridge");
}

int main()
{
    Circles circles;

    circles.size = 0;
    circles.capacity = sizeof(VerletCirlce);
    circles.circle = malloc(circles.capacity);

    Chain bridge_links;

    bridge_links.size = 0;
    bridge_links.capacity = sizeof(Link);
    bridge_links.link = malloc(bridge_links.capacity);
    
    init();
    init_bridge(&circles, &bridge_links);

    while(!WindowShouldClose())
    {
        for(int l = 0; l < bridge_links.size; l++)
            maintain_link(&bridge_links.link[l]);
        update_circles(&circles);
        BeginDrawing();
            ClearBackground(BLACK);
            draw_circles(&circles);
            DrawFPS(0, 0);
        EndDrawing();
    }

    free(circles.circle);
    CloseWindow();
    return 0;
}

// add ball when holding down mouse
// collisions with balls
// split link and circle functionality from physics