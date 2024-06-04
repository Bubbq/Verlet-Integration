#include "headers/raylib.h"
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
    for(int i = 0; i < balls->size; i++)
    {
        Ball* ball = &balls->ball[i];
        DrawCircleV(ball->pos, BALL_RADIUS, ball->col);
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

        BeginDrawing();
            ClearBackground(BLACK);
            drawBalls(&balls);
            DrawCircleLinesV(CENTER_POINT, RADIUS, LIME);
        EndDrawing();
    }

    deinit(&balls);
    return 0;
}