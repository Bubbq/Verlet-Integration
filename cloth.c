#include "headers/raylib.h"
#include "headers/raymath.h"
#include <math.h>
#include <stddef.h>

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#include <stdlib.h>
#include <stdio.h>

const int R = 10;
const int FPS = 120;
const int SCRW = 750, SCRH = 750;

const Vector2 GRAVITY = { 0, 2000.0f };
const Vector2 CENTER = { (SCRW / 2.0f), (SCRH / 2.0f) };

const float DAMP = 0.950f;

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
	size_t capacity;
	Link* link;
} Chain;

const int ROW = 25;
const int COL = 25;

void init()
{
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

void resize_chain(Chain* chain)
{
	chain->capacity *= 2;
	chain->link = realloc(chain->link, chain->capacity);
}

void add_link(Chain* chain, Link link)
{
	if((chain->size * sizeof(link)) == chain->capacity)
		resize_chain(chain);

	chain->link[chain->size++] = link;
}

int main()
{
	// Link links[ROW][COL];
    VerletCirlce circles[ROW][COL];
	Chain chain = { 0, sizeof(Link), malloc(sizeof(Link)) };

    const int XPAD = 100;
    const int YPAD = 50;

    const int XDIST = ((SCRW - (2 * XPAD)) / (COL - 1));
    const int YDIST = ((SCRH - (2 * YPAD)) / (ROW - 1));

	const int DIAGDIAST = sqrt(pow(XDIST, 2)+ pow(YDIST, 2));

    Vector2 vc_position = { XPAD, YPAD };

    // adding circles
    for(int r = 0; r < ROW; r++, vc_position = (Vector2){ XPAD, (vc_position.y + YDIST) })
        for(int c = 0; c < COL; (vc_position.x += XDIST), c++)
            circles[r][c] = (VerletCirlce){ vc_position, vc_position, GRAVITY, R, RAYWHITE, (r == 0) ? SUSPENDED : FREE };

    // adding links
    for(int r = 0; r < ROW; r++)
    {
        for(int c = 0; c < COL; c++)
        {
            for(int dx = -1; dx <= 1; dx++)
            {
                for(int dy = -1; dy <= 1; dy++)
                {
                    int nr = (r + dx);
                    int nc = (c + dy);

                    if((nr >= ROW || nr < 0) || (nc >= COL || nc < 0) || ((nr == r) && (nc == c))) continue;

					float target_distance;

                    if(r == nr)
						// target_distance = XDIST;
						add_link(&chain, (Link){ &circles[r][c], &circles[nr][nc], XDIST });
                    else if(c == nc)
						// target_distance = YDIST;
						add_link(&chain, (Link){ &circles[r][c], &circles[nr][nc], YDIST });
					// else 
					// 	target_distance = DIAGDIAST;
						
					// add_link(&chain, (Link){ &circles[r][c], &circles[nr][nc], target_distance });
                }
            }
        }
    }

    init();

    while(!WindowShouldClose())
    {
		float dt = (GetFrameTime());

		for(int r = 0; r < ROW; r++)
			for(int c = 0; c < COL; c++)
			{
				// updating position
				if(circles[r][c].status == FREE)
				{
					Vector2 velocity = Vector2Scale(Vector2Subtract(circles[r][c].curr_pos, circles[r][c].old_pos), DAMP);
					circles[r][c].old_pos = circles[r][c].curr_pos;
					circles[r][c].curr_pos = Vector2Add(circles[r][c].curr_pos, Vector2Add(velocity, Vector2Scale(circles[r][c].acceleration, powf(dt, 2.0f))));
				}

				else if(circles[r][c].status == SUSPENDED)
					circles[r][c].curr_pos = (Vector2){ XPAD + (XDIST * c), YPAD };

                // for(int i = 0; i < ROW; i++)
				// {
				// 	for(int j = 0; j < COL; j++)
				// 	{
				// 		if((i == r) && (j == c)) 
                //             continue;

				// 		if(Vector2Distance(circles[r][c].curr_pos, circles[i][j].curr_pos) <= (circles[r][c].radius + circles[i][j].radius))
				// 		{
				// 			float delta = (circles[r][c].radius + circles[i][j].radius) - Vector2Distance(circles[r][c].curr_pos, circles[i][j].curr_pos);
				// 			Vector2 direction = Vector2Normalize(Vector2Subtract(circles[r][c].curr_pos, circles[i][j].curr_pos));

				// 			circles[r][c].curr_pos = Vector2Add(circles[r][c].curr_pos, Vector2Scale(direction, (delta / 2.0f)));
				// 			circles[i][j].curr_pos = Vector2Subtract(circles[i][j].curr_pos, Vector2Scale(direction, (delta / 2.0f)));
				// 		}
				// 	}
				// }
			}

		// maintaining link
		for(int i = 0; i < chain.size; i++)
		{
            if((Vector2Distance(chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos)) >= chain.link[i].target_distance)
            {
                float circle_distance = Vector2Distance(chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos);
                float delta = chain.link[i].target_distance - circle_distance;
                Vector2 n = Vector2Normalize(Vector2Subtract(chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos));

                chain.link[i].vc1->curr_pos = Vector2Add(chain.link[i].vc1->curr_pos, Vector2Scale(n, (delta / 2.0f)));
                chain.link[i].vc2->curr_pos = Vector2Subtract(chain.link[i].vc2->curr_pos, Vector2Scale(n, (delta / 2.0f)));
            }

			// deleting link with mouse
			if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointLine(GetMousePosition(), chain.link[i].vc1->curr_pos, chain.link[i].vc2->curr_pos, 20))
			{
				for(int l = i; l < chain.size; l++)
					chain.link[l] = chain.link[l + 1];
				chain.size--;
			}

		}

        BeginDrawing();
            ClearBackground(BLACK);

			// draw links
			for(int i = 0; i < chain.size; i++)
				DrawLine(chain.link[i].vc1->curr_pos.x, chain.link[i].vc1->curr_pos.y, chain.link[i].vc2->curr_pos.x, chain.link[i].vc2->curr_pos.y, RAYWHITE);
			DrawFPS(0, 0);
		EndDrawing();
    }
    
    return 0;    
}