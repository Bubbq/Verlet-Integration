#include "headers/physics.h"
#include "headers/raylib.h"
#include "headers/raymath.h"
#include <stdlib.h>

void handle_border_collision(VerletCirlce* circle, Vector2 constraint_center, Vector2 world_gravity, float constraint_radius)
{
    if((Vector2Distance(circle->current_position, constraint_center) + circle->radius) >= constraint_radius)
    {
        Vector2 direction = Vector2Normalize(Vector2Subtract(circle->current_position, constraint_center));
        
        circle->current_position = Vector2Add(constraint_center, Vector2Scale(direction, (constraint_radius - circle->radius)));
        circle->acceleration = world_gravity;
    }
}

void handle_verlet_circle_collision(VerletCirlce* circle1, VerletCirlce* circle2)
{
    const float SCALE = 0.45f;
    float distance = Vector2Distance(circle1->current_position, circle2->current_position);
    
    if(CheckCollisionCircles(circle1->current_position, circle1->radius, circle2->current_position, circle2->radius))
    {
        float delta = (circle1->radius + circle2->radius) - distance;
        Vector2 direction = Vector2Normalize(Vector2Subtract(circle1->current_position, circle2->current_position));

        if(circle1->status == FREE)
            circle1->current_position = Vector2Add(circle1->current_position, Vector2Scale(direction, (delta * SCALE)));
        if(circle2->status == FREE)
            circle2->current_position = Vector2Subtract(circle2->current_position, Vector2Scale(direction, (delta * SCALE)));

        // TODO: collision makes you have worlds gravity
    }
}

void update_position(VerletCirlce * circle, float slow_down_scale, float dt)
{
    const int MAX_V = 20.0f;

    Vector2 velocity = Vector2Scale(Vector2Subtract(circle->current_position, circle->previous_position), slow_down_scale);
    
    if(Vector2Length(velocity) >= MAX_V) 
        velocity = (Vector2){};

    circle->previous_position = circle->current_position;

    // x(n+1) = x(n) + v + a(dt)^2, verlet integration formula
    circle->current_position = Vector2Add(circle->current_position, Vector2Add(velocity, Vector2Scale(circle->acceleration, powf(dt, 2.0f))));
}

void apply_gravity(VerletCirlce* circle, Vector2 world_gravity, float dt)
{
    Vector2 delta = Vector2Scale(Vector2Subtract(world_gravity, circle->acceleration), dt);
    circle->acceleration = Vector2Add(circle->acceleration, delta);
}

void add_verlet_circle(Circles* circles, VerletCirlce circle)
{
    if((circles->size * sizeof(VerletCirlce)) == circles->capacity)
        resize_circles(circles);

    circles->circle[circles->size++] = circle;
}

void remove_verlet_circle(Circles* circles, int position)
{
    for(int i = position; i < circles->size; i++)
        circles->circle[i] = circles->circle[i + 1];

    circles->size--;
}

void resize_circles(Circles* circles)
{   
    circles->capacity *= 2;
    circles->circle = realloc(circles->circle, circles->capacity);
}

void draw_circles(Circles* circles)
{
    int c = 0;
    for(VerletCirlce* vc = circles->circle; c < circles->size; c++, vc = (circles->circle + c))
        DrawCircleSector(vc->current_position, vc->radius, 0, 360, 1, vc->color);
}

void maintain_link(Link* link)
{
    const float SCALE = 0.30;
    float circle_distance = Vector2Distance(link->circle1->current_position, link->circle2->current_position);

    if(circle_distance >= link->target_distance)
    {
        float delta = link->target_distance - circle_distance;
        Vector2 direction = Vector2Normalize(Vector2Subtract(link->circle1->current_position, link->circle2->current_position));

        if(link->circle1->status == FREE)
            link->circle1->current_position = Vector2Add(link->circle1->current_position, Vector2Scale(direction, (delta * SCALE)));
        
        if(link->circle2->status == FREE)
            link->circle2->current_position = Vector2Subtract(link->circle2->current_position, Vector2Scale(direction, (delta * SCALE)));
    }
}

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

void resize_chain(Chain* chain)
{
    chain->capacity *= 2;
    chain->link = realloc(chain->link, chain->capacity);
}

void draw_links(Chain* chain)
{
    for(int i = 0; i < chain->size; i++)
        DrawLine(chain->link[i].circle1->current_position.x, chain->link[i].circle1->current_position.y, chain->link[i].circle2->current_position.x, chain->link[i].circle2->current_position.y, LIGHTGRAY);
}