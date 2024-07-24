#include "headers/raylib.h"
#include "headers/raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#include <stdlib.h>
#include <stdio.h>

const int FPS = 60;
const int SCRW = 750, SCRH = 750;

const int ROW = 100, COL = 100;

const int YPAD = 30;
const int XPAD = 100;

const int XDIST = ((SCRW - (2 * XPAD)) / (COL - 1));
const int YDIST = ((SCRH - (2 * YPAD)) / (ROW - 1));

typedef enum
{
    FREE = 0,
    SUSPENDED = 1,
} Status;

typedef struct
{
    Vector2 curr_pos;
    Vector2 old_pos;
    Vector2 acceleration;
    float radius;
    Color color;
    Status status;
} VerletCirlce;

typedef struct
{
    VerletCirlce* vc1;
    VerletCirlce* vc2;
    float target_distance;
} Link;

typedef struct
{
	int size;
	size_t capacity;
	Link* link;
} Chain;

typedef struct
{
    int size;
    size_t capacity;
    VerletCirlce* circle;
} Circles;

void resize_chain(Chain* chain)
{
	chain->capacity *= 2;
	chain->link = realloc(chain->link, chain->capacity);
}

void resize_circles(Circles* c)
{
    c->capacity *= 2;
    c->circle = realloc(c->circle, c->capacity);
}

void add_link(Chain* chain, Link link)
{
	if((chain->size * sizeof(link)) == chain->capacity)
		resize_chain(chain);

	chain->link[chain->size++] = link;
}

void add_verlet_circle(Circles* circles, VerletCirlce vc)
{
    if((circles->size * sizeof(VerletCirlce)) == circles->capacity) 
        resize_circles(circles);

    circles->circle[circles->size++] = vc;
}

void remove_link(Chain* chain, int pos)
{
    for(int l = pos; l < chain->size; l++)
					chain->link[l] = chain->link[l + 1];
    chain->size--;
}

void update_position(VerletCirlce * vc)
{
    const float DAMP = 0.950f;
    Vector2 velocity = Vector2Scale(Vector2Subtract(vc->curr_pos, vc->old_pos), DAMP);
    vc->old_pos = vc->curr_pos;
    vc->curr_pos = Vector2Add(vc->curr_pos, Vector2Add(velocity, Vector2Scale(vc->acceleration, powf(GetFrameTime(), 2.0f))));
}

void update_circles(Circles* circles)
{
    int i = 0;
    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i))
    {
        switch(vc->status)
        {
            case FREE: update_position(vc); break;  
            case SUSPENDED: vc->curr_pos = (Vector2){ XPAD + (XDIST * (i % (COL))), YPAD }; break;
            default:
                break;
        }

        if(vc->curr_pos.y >= SCRH + vc->radius)
            vc->curr_pos.y = SCRH + vc->radius;
    }
}

void draw_circles(Circles* circles)
{
    int i = 0;
    for(VerletCirlce* vc = circles->circle; i < circles->size; i++, vc = (circles->circle + i))
        DrawCircleSector(vc->curr_pos, vc->radius, 0, 360, 2, vc->color);
}

void update_links(Chain* chain)
{
    const float SCALE = 0.35f;

    for(int i = 0; i < chain->size; i++)
    {
        // maintaining link distance
        if((Vector2Distance(chain->link[i].vc1->curr_pos, chain->link[i].vc2->curr_pos)) >= chain->link[i].target_distance)
        {
            float circle_distance = Vector2Distance(chain->link[i].vc1->curr_pos, chain->link[i].vc2->curr_pos);
            float delta = chain->link[i].target_distance - circle_distance;
            Vector2 n = Vector2Normalize(Vector2Subtract(chain->link[i].vc1->curr_pos, chain->link[i].vc2->curr_pos));

            chain->link[i].vc1->curr_pos = Vector2Add(chain->link[i].vc1->curr_pos, Vector2Scale(n, (delta * SCALE)));
            chain->link[i].vc2->curr_pos = Vector2Subtract(chain->link[i].vc2->curr_pos, Vector2Scale(n, (delta * SCALE)));
        }
        
        // deleting link with mouse
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointLine(GetMousePosition(), chain->link[i].vc1->curr_pos, chain->link[i].vc2->curr_pos, 15))
            remove_link(chain, i);
    }   
}

void draw_links(Chain* chain)
{
    for(int i = 0; i < chain->size; i++)
        DrawLine(chain->link[i].vc1->curr_pos.x, chain->link[i].vc1->curr_pos.y, chain->link[i].vc2->curr_pos.x, chain->link[i].vc2->curr_pos.y, RAYWHITE);
}

void init_circles(Circles* circles)
{
    const int RADIUS = 5;
    const Vector2 GRAVITY = { 0, 3000.0f };

    Vector2 vc_position = { XPAD, YPAD };
    
    for(int r = 0; r < ROW; r++, vc_position = (Vector2){ XPAD, (vc_position.y + YDIST) })
        for(int c = 0; c < COL; (vc_position.x += XDIST), c++)
            add_verlet_circle(circles, (VerletCirlce){ vc_position, vc_position, GRAVITY, RADIUS, RAYWHITE, (r == 0) ? SUSPENDED : FREE });
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

int main()
{
	Chain chain = { 0, sizeof(Link), malloc(sizeof(Link)) };
    Circles circles = { 0, sizeof(VerletCirlce), malloc(sizeof(VerletCirlce)) };

    init(&circles, &chain);

    while(!WindowShouldClose())
    {
        update_links(&chain);
        update_circles(&circles);

        BeginDrawing();
            ClearBackground(BLACK);
			draw_links(&chain);
			DrawFPS(0, 0);
		EndDrawing();
    }

    return 0;    
}

// TODO: 'grab' function, able to make row & col diff numbers split code in different programs with verlet physics 