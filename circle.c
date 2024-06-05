#include "headers/raylib.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

const int FPS = 60;
const int RADIUS = 200;
const int MAX_SPEED = 45;
const int MIN_SPEED = 15;
const int BALL_RADIUS = 10;
const int SCREEN_SIZE = 700;
const Vector2 CENTER_POINT = {(SCREEN_SIZE / 2.0f), (SCREEN_SIZE / 2.0f)};

const float GRAV = 500.0f;

// how many seconds a single frame lasts
float frame_time;

typedef struct
{
    Vector2 pos;
    float vx, vy;
    Color col;
} Ball;

typedef struct
{
    size_t cap;
    int size;
    Ball* ball;
} Balls;

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

    else 
    {
        bool collide;
        Vector2 pos;
       
        do
        {
            collide = false;
            pos = getRandomCirclePosition();

            for(int i = 0; i < balls->size; i++)
            {
                Ball* ball = &balls->ball[i];
                if(CheckCollisionCircles(pos, BALL_RADIUS, ball->pos, BALL_RADIUS)) collide = true;
            }
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

        balls->ball[balls->size++] = (Ball){pos, vx, 0, col};
    }
}

void drawBalls(Balls* balls)
{
    for(int i = 0; i < balls->size; i++) DrawCircleV(balls->ball[i].pos, BALL_RADIUS, balls->ball[i].col);
}

void moveBalls(Balls* balls) {
    for (int i = 0; i < balls->size; i++)
    {
        Ball* ball = &balls->ball[i];

        // ball velocity grows by GRAV every second
        ball->vy += GRAV * frame_time;

        // ball moves by vx/vy px every second
        ball->pos.y += ball->vy * frame_time;
        ball->pos.x += ball->vx * frame_time;

        // find the distnace from the center of main circle
        float dist = Vector2Distance(ball->pos, CENTER_POINT);

        // handle collision when ball touches the edge, refer to VECTOR REFLECTION FORMULA
        if (dist + BALL_RADIUS >= RADIUS)
        {
            // v' = v -2(v * n)n, where v is current vect, and n is normal/unit vect, and v' is new current vect
            float ba = atan2f((ball->pos.y - CENTER_POINT.y), (ball->pos.x - CENTER_POINT.x));
            
            float nx = cosf(ba);
            float ny = sinf(ba);

            // (v * n), * is dot product, or vx * nx + vy * ny
            float dp = (ball->vx * nx) + (ball->vy * ny);

            ball->vx -= 2.0f * dp * nx;
            ball->vy -= 2.0f * dp * ny;

            // so the coordinates of the ball is based of a unit circle that has a radius of radius - ball_radius 
            ball->pos.x = CENTER_POINT.x + (RADIUS - BALL_RADIUS) * nx;
            ball->pos.y = CENTER_POINT.y + (RADIUS - BALL_RADIUS) * ny;
        }
    }
}

void init(Balls* balls)
{
    SetTargetFPS(FPS);
    srand(time(NULL));
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
        frame_time = GetFrameTime();
        moveBalls(&balls);
       
        BeginDrawing();
            ClearBackground(BLACK);
            drawBalls(&balls);
            DrawCircleLinesV(CENTER_POINT, RADIUS, LIME);
        EndDrawing();
    }

    deinit(&balls);
    return 0;
}