#include "headers/raylib.h"
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

const int FPS = 60;
const int SCREEN_SIZE = 700;

const int MIN_SPEED = 15;
const int MAX_SPEED = 45;
const int BALL_RADIUS = 10;

const int RADIUS = 200;
const Vector2 CENTER_POINT = {(SCREEN_SIZE / 2.0f), (SCREEN_SIZE / 2.0f)};

const float GRAV = 1000.0f;
const float ACTIVE_TIME = 3.0f;

// how many seconds a single frame lasts
float frame_time = (1.0f / FPS);

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
    Timer timer;
} Ball;

typedef struct
{
    size_t cap;
    int size;
    Ball* ball;
} Balls;

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

void addBall(Balls* balls)
{
    if(balls->size * sizeof(Ball) == balls->cap) resizeBalls(balls);

    bool collide;
    Vector2 pos;

    // random position not colliding w any ball    
    do
    {
        collide = false;
        pos = getRandomCirclePosition();
        for(int i = 0; i < balls->size; i++) if(CheckCollisionCircles(pos, BALL_RADIUS, balls->ball[i].pos, BALL_RADIUS)) collide = true;
    } while(collide);

    // random speed
    float vx = GetRandomValue(MIN_SPEED, MAX_SPEED);

    // random color
    Color col;
    int randcol = GetRandomValue(1, 6);

    switch (randcol)
    {
        case 1: col = RED; break;
        case 2: col = ORANGE; break;
        case 3: col = YELLOW; break;
        case 4: col = GREEN; break;
        case 5: col = BLUE; break;
        case 6: col = VIOLET; break;
        default:
            break;
    }
    
    // add ball to list
    balls->ball[balls->size] = (Ball){pos, vx, 0, col, true};

    // start duration of that ball
    StartTimer(&balls->ball[balls->size].timer, ACTIVE_TIME);
    
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

void handleBallCollision(Balls* balls, Ball* ball)
{
    for(int j = 0; j < balls->size; j++)
    {
        // only unactive balls are frozen in place
        Ball* ball2 = &balls->ball[j];
        if(ball2->active) continue;
        
        if(CheckCollisionCircles(ball->pos, BALL_RADIUS, ball2->pos, BALL_RADIUS)) collide(ball, ball2->pos, (BALL_RADIUS * 2.0f));
    }
}

void moveBalls(Balls* balls) {
    for (int i = 0; i < balls->size; i++)
    {
        // only care about active balls
        Ball* ball = &balls->ball[i];
        ball->active = !TimerDone(ball->timer);
        if(!ball->active) continue;

        // ball velocity grows by GRAV every second
        ball->vy += GRAV * frame_time;

        // ball moves by vx/vy pixels every second
        ball->pos.y += ball->vy * frame_time;
        ball->pos.x += ball->vx * frame_time;
        
        // edge of circle collision
        if (Vector2Distance(ball->pos, CENTER_POINT) + BALL_RADIUS >= RADIUS) collide(ball, CENTER_POINT, (RADIUS - BALL_RADIUS));

        // ball collision
        handleBallCollision(balls, ball);
    }
}

void init(Balls* balls)
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Circle");

    balls->size = 0;
    balls->cap = 25 * sizeof(Ball);
    balls->ball = malloc(balls->cap);
}

void deinit(Balls* balls)
{
    free(balls->ball);
    CloseWindow();
}

int main()
{
    Balls balls;
    init(&balls);

    while(!WindowShouldClose())
    {
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) addBall(&balls);
        moveBalls(&balls);

        BeginDrawing();
            drawBalls(&balls);
            DrawFPS(0, 0);
            ClearBackground(BLACK);
            DrawCircleLinesV(CENTER_POINT, RADIUS, LIME);
        EndDrawing();
    }

    deinit(&balls);
    return 0;
}