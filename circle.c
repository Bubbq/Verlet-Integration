#include "headers/raylib.h"
#include <raymath.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CHAR_LIMIT 1024

#define FPS 60
#define RADIUS 200
#define SCREEN_SIZE 700

#define MIN_SPEED 15
#define MAX_SPEED 45
#define BALL_RADIUS 10

#define GRAVITY 1000.0f
#define ADD_TIME 3.0f

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

const Vector2 CENTER_POINT = {SCREEN_SIZE / 2.0f, SCREEN_SIZE / 2.0f};
const float FRAME_TIME = (1.0f / FPS);
const size_t INIT_CAP = (25 * sizeof(Ball));

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

void resizeBalls(Balls* balls)
{
    balls->cap *= 2;
    balls->ball = realloc(balls->ball, balls->cap);
}

Color randomColor()
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

void addBall(Balls* balls, Timer* timer)
{
    if(balls->size * sizeof(Ball) == balls->cap) resizeBalls(balls);

    // random position not colliding w any ball    
    bool collide;
    Vector2 pos;
    do
    {
        collide = false;
        pos = getRandomCirclePosition();
        for(int i = 0; i < balls->size; i++) if(CheckCollisionCircles(pos, BALL_RADIUS, balls->ball[i].pos, BALL_RADIUS)) collide = true;
    } while(collide);

    // freeze current ball
    balls->ball[balls->size - 1].active = false;
    
    // add new ball to list
    balls->ball[balls->size] = (Ball){pos, GetRandomValue(MIN_SPEED, MAX_SPEED), 0, randomColor(), true};

    // start add timer
    StartTimer(timer, ADD_TIME);
    
    balls->size++;
}

void drawBalls(Balls* balls)
{
    for(int i = 0; i < balls->size; i++) DrawCircleV(balls->ball[i].pos, BALL_RADIUS, balls->ball[i].col);
}

// uses VECTOR REFLECTION FORMULA, v' = v -2(v * n)n, to handle collision between a ball at some point
void collide(Ball* ball, Vector2 collision_pos, float radius)
{
    // collision angle (in radians)
    float ca = atan2f((ball->pos.y - collision_pos.y), (ball->pos.x - collision_pos.x));

    // components of normal vector
    float nx = cosf(ca);
    float ny = sinf(ca);

    // (v * n), dot product, or vx * nx + vy * ny
    float dp = (ball->vx * nx) + (ball->vy * ny);

    // calculating v'
    ball->vx -= (2 * dp * nx);
    ball->vy -= (2 * dp * ny);

    // position correction
    ball->pos.x = collision_pos.x + (radius * nx);
    ball->pos.y = collision_pos.y + (radius * ny);
}

void handleBallCollision(Balls* all_balls, Ball* ball)
{
    for(int j = 0; j < all_balls->size; j++)
    {
        // unactive balls are frozen in place
        Ball* ball2 = &all_balls->ball[j];
        if(ball2->active) continue;
        
        if(CheckCollisionCircles(ball->pos, BALL_RADIUS, ball2->pos, BALL_RADIUS)) collide(ball, ball2->pos, (BALL_RADIUS * 2.0f));
    }
}

void moveBall(Balls* all_balls, Ball* ball)
{
    // ball velocity grows by GRAV every second
    ball->vy += (GRAVITY * FRAME_TIME);

    // ball moves by vx/vy pixels every second
    ball->pos.y += (ball->vy * FRAME_TIME);
    ball->pos.x += (ball->vx * FRAME_TIME);

    // ball collision
    handleBallCollision(all_balls, ball);
    
    // edge of circle collision
    if (Vector2Distance(ball->pos, CENTER_POINT) + BALL_RADIUS >= RADIUS) collide(ball, CENTER_POINT, (RADIUS - BALL_RADIUS));
}

void init(Balls* balls)
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Circle");
    
    balls->size = 0;
    balls->cap = INIT_CAP;
    balls->ball = malloc(balls->cap);
}

void deinit(Ball* alloc_ball_mem)
{
    free(alloc_ball_mem);
    CloseWindow();
}

int main()
{
    Balls balls;
    Timer add_timer;
    char ball_count[CHAR_LIMIT];
    
    init(&balls);
    
    while(!WindowShouldClose())
    {
        // moves the current ball
        if(balls.size > 0) moveBall(&balls, &balls.ball[balls.size - 1]);
        
        // adds ball every ADD_TIME seconds
        if(TimerDone(add_timer)) 
        {
            addBall(&balls, &add_timer);
            sprintf(ball_count, "BALL COUNT: %d", balls.size);
        }

        BeginDrawing();
            drawBalls(&balls);
            DrawFPS(0, 0);
            ClearBackground(BLACK);
            DrawCircleLinesV(CENTER_POINT, RADIUS, LIME);
            DrawText(ball_count, 0, 15, 19, LIME);
        EndDrawing();
    }

    deinit(balls.ball);
    return 0;
}