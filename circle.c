#include "headers/raylib.h"
#include <math.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#define CHAR_LIMIT 1024

typedef struct
{
   double startTime;
   double lifeTime;
} Timer;

typedef struct
{
    Vector2 pos;
    float vx, vy;
    Color col;
    bool active;
} Ball;

typedef struct
{
    size_t cap;
    int size;
    Ball* ball;
} Balls;

enum Gamestate
{
   ACTIVE = 0,
   DONE = 1,
   READY = 2,
   QUIT = 3,
};

const int RADIUS = 150;
const int SCREEN_SIZE = 700;

const float ADD_TIME = 3.0f;
const float GRAVITY = 1000.0f;

const int MIN_SPEED = 15;
const int MAX_SPEED = 45;
const int BALL_RADIUS = 10;

const int FPS = 60;
const float FRAME_TIME = (1.0f / FPS);

const Vector2 CENTER_POINT = {SCREEN_SIZE / 2.0f, SCREEN_SIZE / 2.0f};
const size_t INIT_CAP = (25 * sizeof(Ball));

Balls all_balls;
Timer add_timer;
enum Gamestate game_state;

void StartTimer(Timer *timer, double lifetime)
{
   timer->startTime = GetTime();
   timer->lifeTime = lifetime;
}

bool TimerDone(Timer timer)
{
   return GetTime() - timer.startTime >= timer.lifeTime;
}

Vector2 getRandomCirclePosition()
{
    // get random angle
    float theta = GetRandomValue(0, 360);
    
    // random distance from center point of main circle
    float r = GetRandomValue(1, RADIUS - (BALL_RADIUS * 2.0f));
    
    // get x and y distance of that angle in proportion to r
    float x  = CENTER_POINT.x + (cos(theta * DEG2RAD) * r);
    float y  = CENTER_POINT.y + (sin(theta * DEG2RAD) * r);

    return (Vector2){x, y};
}

Color getRandomColor()
{
    switch (GetRandomValue(0, 8))
    {
        case 0: return WHITE; break;
        case 1: return RED; break;
        case 2: return ORANGE; break;
        case 3: return YELLOW; break;
        case 4: return GREEN; break;
        case 5: return BLUE; break;
        case 6: return VIOLET; break;
        case 7: return BROWN; break;
        case 8: return PINK; break;
        default: return SKYBLUE; break;
    }
}

float getAngle(Vector2 v1, Vector2 v2)
{
	float angle = atan2f((v1.y - v2.y), (v1.x - v2.x)) * RAD2DEG;
	
	if(angle > 360) angle -= 360;
	else if(angle < 0) angle += 360;

	return angle;
}

void resizeBalls()
{
    all_balls.cap *= 2;
    all_balls.ball = realloc(all_balls.ball, all_balls.cap);
}

void resetBalls()
{
    all_balls.size = 0;
    all_balls.cap = INIT_CAP;
    free(all_balls.ball);
}

void addBall()
{
    if(all_balls.size * sizeof(Ball) == all_balls.cap) resizeBalls();

    // random position not colliding w any ball    
    bool collide;
    Vector2 pos;
    do
    {
        collide = false;
        pos = getRandomCirclePosition();
        for(int i = 0; i < all_balls.size; i++) if(CheckCollisionCircles(pos, BALL_RADIUS, all_balls.ball[i].pos, BALL_RADIUS)) collide = true;
    } while(collide);

    // freeze current ball
    all_balls.ball[all_balls.size - 1].active = false;
    
    // add new ball to list
    all_balls.ball[all_balls.size++] = (Ball){pos, GetRandomValue(MIN_SPEED, MAX_SPEED), 0, getRandomColor(), true};

    StartTimer(&add_timer, ADD_TIME);
}

void drawBalls()
{
    for(int i = 0; i < all_balls.size; i++) DrawCircleV(all_balls.ball[i].pos, BALL_RADIUS, all_balls.ball[i].col);
}

// uses VECTOR REFLECTION FORMULA, v' = v -2(v * n)n, to handle collision between a ball at some point
void collide(Ball* ball, Vector2 collision_pos, float radius)
{
    // collision angle
    float ca = getAngle(ball->pos, collision_pos);

    // the unit vector of collision 
    Vector2 n = {cosf(ca * DEG2RAD), sinf(ca * DEG2RAD)};

    // (v * n), the dot product between the current velocity and the normal vector
    float dp = Vector2DotProduct((Vector2){ball->vx, ball->vy}, n);

    // calculating v'
    ball->vx -= (2 * dp * n.x);
    ball->vy -= (2 * dp * n.y);

    // position correction
    ball->pos.x = collision_pos.x + (radius * n.x);
    ball->pos.y = collision_pos.y + (radius * n.y);
}

void handleBallCollision(Ball* ball)
{
    for(int i = 0; i < all_balls.size; i++)
    {
        // prevent colliding with itself
        Ball* ball2 = &all_balls.ball[i];
        if(Vector2Equals(ball->pos, ball2->pos)) continue;
        
        if(CheckCollisionCircles(ball->pos, BALL_RADIUS, ball2->pos, BALL_RADIUS)) collide(ball, ball2->pos, (BALL_RADIUS * 2.0f));
    }
}

void handleBorderCollision(Ball* ball)
{
    float ca = getAngle(ball->pos, CENTER_POINT);

    if (Vector2Distance(ball->pos, CENTER_POINT) + BALL_RADIUS >= RADIUS)
    {
        if(ca <= 315) collide(ball, CENTER_POINT, (RADIUS - BALL_RADIUS));

        // ball leaves the sector, the game is done
        else game_state = DONE;
    }
}

void moveBall(Ball* ball)
{
    // ball velocity grows by GRAV every second
    ball->vy += (GRAVITY * FRAME_TIME);

    // ball moves by vx/vy pixels every second
    ball->pos.y += (ball->vy * FRAME_TIME);
    ball->pos.x += (ball->vx * FRAME_TIME);

    // ball collision
    handleBallCollision(ball);
    
    // ball hitting the circle's edges
    handleBorderCollision(ball);
}

void init()
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Circle");
    
    all_balls.size = 0;
    all_balls.cap = INIT_CAP;
    all_balls.ball = malloc(all_balls.cap);
    
    game_state = ACTIVE;
}

void deinit(Ball* alloc_ball_mem)
{
    if(all_balls.size > 0) free(alloc_ball_mem);
    CloseWindow();
}

int main()
{
    init();
    char ball_count_dialouge[CHAR_LIMIT] = "";

    while((game_state != QUIT) && !WindowShouldClose())
    {
        int cidx = (all_balls.size == 0) ? 0 : all_balls.size - 1;
        Ball* current_ball = &all_balls.ball[cidx];
       
        BeginDrawing();
            ClearBackground(BLACK);

            DrawCircleSectorLines(CENTER_POINT, RADIUS, 0, 315, 1, LIME);
            DrawCircleV(CENTER_POINT, RADIUS - 1, BLACK);

            DrawText(ball_count_dialouge, 0, 15, 19, LIME);
            DrawFPS(0, 0);
            drawBalls();

            sprintf(ball_count_dialouge, "BALL COUNT: %d", all_balls.size);

            switch (game_state)
            {
                case ACTIVE:
                    if(TimerDone(add_timer))
                    {
                        addBall();
                        StartTimer(&add_timer, ADD_TIME);
                    }
                    if(all_balls.size > 0) moveBall(current_ball);
                    break;
                case DONE:
                    if(GuiButton((Rectangle){(SCREEN_SIZE / 2.0f) - 50, (SCREEN_SIZE - 150), 100, 50}, "OK")) 
                    {
                        resetBalls();
                        game_state = READY;
                    }
                    break;
                case READY: 
                    // TODO: display wether player won or not
                    // TODO: code for placing bet
                    // button to start the game
                    if(GuiButton((Rectangle){(SCREEN_SIZE / 2.0f) - 50, (SCREEN_SIZE - 150), 100, 50}, "START")) 
                    {
                        game_state = ACTIVE;
                    }
                    break;
                default:
                    break;
            }

        EndDrawing();
    }

    deinit(all_balls.ball);
    return 0;
}