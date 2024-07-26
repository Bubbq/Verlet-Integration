#include "headers/physics.h"
#include "headers/raylib.h"
#include "headers/raymath.h"
#include <stdlib.h>

// void handle_verlet_circle_collision(VerletCirlce* circle1, VerletCirlce* circle2)
// {
//     const float SCALE = 0.45f;
//     float distance = Vector2Distance(circle1->current_position, circle2->current_position);
    
//     if(CheckCollisionCircles(circle1->current_position, circle1->radius, circle2->current_position, circle2->radius))
//     {
//         float delta = (circle1->radius + circle2->radius) - distance;
//         Vector2 direction = Vector2Normalize(Vector2Subtract(circle1->current_position, circle2->current_position));

//         circle1->current_position = Vector2Add(circle1->current_position, Vector2Scale(direction, (delta * SCALE)));
//         circle2->current_position = Vector2Subtract(circle2->current_position, Vector2Scale(direction, (delta * SCALE)));
//     }
// }

// void handle_border_collision(VerletCirlce* circle, Vector2 constraint_center, float constraint_radius)
// {
//     if(Vector2Distance(circle->current_position, constraint_center) + circle->radius >= constraint_radius)
//     {
//         circle->current_position = Vector2Scale(Vector2Subtract(circle->current_position, constraint_center), (constraint_radius - circle->radius));
//         circle->acceleration = Vector2Scale(circle->acceleration, -1);
//     }
// }

void update_position(VerletCirlce * circle, float dt)
{
    const float SLOWDOWN_SCALE = 0.950f;
    const int MAX_V = 10.0f;

    Vector2 velocity = Vector2Scale(Vector2Subtract(circle->current_position, circle->previous_position), SLOWDOWN_SCALE);
    if(Vector2Length(velocity) >= MAX_V) velocity = (Vector2){};

    circle->previous_position = circle->current_position;

    // x(n+1) = x(n) + v + a(dt)^2, verlet integration formula
    circle->current_position = Vector2Add(circle->current_position, Vector2Add(velocity, Vector2Scale(circle->acceleration, powf(dt, 2.0f))));
}

void apply_gravity(VerletCirlce* circle, Vector2 world_gravity)
{
    Vector2 delta = Vector2Scale(Vector2Subtract(world_gravity, circle->acceleration), GetFrameTime());
    circle->acceleration = Vector2Add(circle->acceleration, delta);
}

void resize_circles(Circles* circles)
{   
    circles->capacity *= 2;
    circles->circle = realloc(circles->circle, circles->capacity);
}

void add_verlet_circle(Circles* circles, VerletCirlce circle)
{
    if((circles->size * sizeof(VerletCirlce)) == circles->capacity)
        resize_circles(circles);

    circles->circle[circles->size++] = circle;
}

// void remove_verlet_circle(Circles* circles, int position)
// {
//     for(int i = position; i < circles->size; i++)
//         circles->circle[i] = circles->circle[i + 1];

//     circles->size--;
// }

void add_link(Chain* chain, Link link)
{
    if((chain->size * sizeof(Link)) == chain->capacity)
        resize_chain(chain);

    chain->link[chain->size++] = link;
}

void remove_link(Chain* chain, int position)
{
    for(int i = position; i < chain->size; i++)
        chain->link[i] = chain->link[i + 1];

    chain->size--;
}

void maintain_link(Link* link)
{
    const float SCALE = 0.30f;
    float circle_distance = Vector2Distance(link->circle1->current_position, link->circle2->current_position);

    if(circle_distance >= link->target_distance)
    {
        float delta = link->target_distance - circle_distance;
        Vector2 direction = Vector2Normalize(Vector2Subtract(link->circle1->current_position, link->circle2->current_position));

        link->circle1->current_position = Vector2Add(link->circle1->current_position, Vector2Scale(direction, (delta * SCALE)));
        link->circle2->current_position = Vector2Subtract(link->circle2->current_position, Vector2Scale(direction, (delta * SCALE)));
    }
}

void resize_chain(Chain* chain)
{
    chain->capacity *= 2;
    chain->link = realloc(chain->link, chain->capacity);
}