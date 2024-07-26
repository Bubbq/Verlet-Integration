#include "headers/raylib.h"
#include "headers/raymath.h"
#include "headers/physics.h"
#include <stdlib.h>

const int SCRW = 900, SCRH = 900;
const int FPS = 60;

const int ROW = 50, COL = 50;
const int RADIUS = 5;

const int XPAD = 100;
const int YPAD = 30;

const int XDIST = ((SCRW - (2 * XPAD)) / (COL - 1));
const int YDIST = 1;

const float MAX_LINK_DIST = 300.0f;

const Vector2 WORLD_GRAVITY = { 0, 2000.0f };

void update_circles(Circles* circles, int* grabbed_link_pos)
{
    const float DAMP = 0.980f;
    int i = 0;

    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i))
    {
        if(CheckCollisionPointCircle(GetMousePosition(), vc->current_position, vc->radius) && !(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)))
            *grabbed_link_pos = i;
        
        apply_gravity(vc, WORLD_GRAVITY, GetFrameTime());

        switch(vc->status)
        {
            case FREE: 
                update_position(vc, DAMP, GetFrameTime()); 
                break;  

            case SUSPENDED: 
                vc->current_position = (Vector2){ XPAD + (XDIST * (i % (COL))), YPAD };
                break;
        }

        if(vc->current_position.y >= SCRH + vc->radius)
            vc->current_position.y = SCRH + vc->radius;
    }
}

void update_links(Chain* chain)
{
    for(int i = 0; i < chain->size; i++)
    {
        Vector2 starting_position = chain->link[i].circle1->current_position, 
                ending_position = chain->link[i].circle2->current_position;
        
        float circle_distance = Vector2Distance(starting_position, ending_position);

        // snapping chain if distance is too far or ripping cloth with mouse
        if((circle_distance >= MAX_LINK_DIST) ||
          ((IsMouseButtonDown(MOUSE_BUTTON_LEFT)) && (CheckCollisionPointLine(GetMousePosition(), starting_position, ending_position, 5))))
        {
            remove_link(chain, i);
        }
        
        maintain_link(&chain->link[i]);
    }   
}

void draw_links(Chain* chain)
{
    for(int i = 0; i < chain->size; i++)
        DrawLine(chain->link[i].circle1->current_position.x, chain->link[i].circle1->current_position.y, chain->link[i].circle2->current_position.x, chain->link[i].circle2->current_position.y, LIGHTGRAY);
}

void init_circles(Circles* circles)
{
    const int RADIUS = 5;
    
    Vector2 vc_position = { XPAD, YPAD };
    
    for(int r = 0; r < ROW; r++, vc_position = (Vector2){ XPAD, (YDIST * r) })
        for(int c = 0; c < COL; (vc_position.x += XDIST), c++)
            add_verlet_circle(circles, (VerletCirlce){ RAYWHITE, RADIUS, (r == 0 ) ? SUSPENDED : FREE, (Vector2){}, vc_position, vc_position, });
}

void init_chain(Chain* chain, Circles* circles)
{
    for(int r = 0; r < ROW; r++)
    {
        for(int c = 0; c < COL; c++)
        {
            // one dim index
            int i_index = (r * ROW) + c;

            for(int dx = -1; dx <= 1; dx++)
            {
                for(int dy = -1; dy <= 1; dy++)
                {
                    // neighboring row and columns
                    int nr = (r + dx);
                    int nc = (c + dy);
                    int j_index = (nr * ROW) + nc; 
                    
                    if((nr >= ROW || nr < 0) || (nc >= COL || nc < 0) || ((nr == r) && (nc == c))) continue;

                    if(r == nr)
					    add_link(chain, (Link){ (circles->circle + i_index), (circles->circle + j_index), XDIST });

                    else if(c == nc)
    					add_link(chain, (Link){ (circles->circle + i_index), (circles->circle + j_index), YDIST });
                }
            }
        }
    }
}

void init(Circles* circles, Chain* chain)
{
    init_circles(circles);
    init_chain(chain, circles);

    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRW, SCRH, "Cloth Demo");
}

void deinit(VerletCirlce* alloc_circle_mem, Link* alloc_link_mem)
{
    free(alloc_circle_mem);
    free(alloc_link_mem);
    CloseWindow();
}

void pull_cloth(Circles* circles, int current_link)
{
    if((current_link != -1) && (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))) 
        circles->circle[current_link].current_position = GetMousePosition(); 
}

int main()
{
	Chain chain = { 0, malloc(sizeof(Link)), sizeof(Link) };
    Circles circles = { 0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce)) };

    int current_link = -1;

    init(&circles, &chain);
    
    while(!WindowShouldClose())
    {
        update_links(&chain);
        update_circles(&circles, &current_link);
        
        pull_cloth(&circles, current_link);

        BeginDrawing();
            ClearBackground(BLACK);
			draw_links(&chain);
            DrawFPS(0, 0);
		EndDrawing();
    }

    return 0;    
}