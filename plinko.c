#include <stdio.h>
#include <stdlib.h>
#include "headers/raylib.h"

typedef struct
{
    bool active;
    float vx, vy;
    Vector2 pos;
} Ball;

typedef struct
{
    int size;
    size_t cap;
    Ball* ball;
} Balls;

const int FPS = 60;
const int RADIUS = 5;
const int LEVELS = 10;
const int SCREEN_SIZE = 700;
const size_t CAP = sizeof(Ball) * 25;

void resizeBalls(Balls* balls)
{
    balls->cap *= 2;
    balls->ball = realloc(balls->ball, balls->cap);
}

void addBall(Balls* balls, Ball nb)
{
    if(balls->size * sizeof(Ball) == balls->cap) resizeBalls(balls);
    balls->ball[balls->size++] = nb;
}

void drawBalls(Balls* balls) {for(int i = 0; i < balls->size; i++) DrawCircleV(balls->ball[i].pos, RADIUS, RAYWHITE);}

void setBoard(Balls* collision_balls)
{
    const int D = 45;
    Vector2 sp = {(SCREEN_SIZE / 2.0f) - D, 100};

    for(int i = 0; i < LEVELS; i++)
    {
        Vector2 cp = sp;
        for(int j = 0; j < (3 + i); j++, cp.x += D) addBall(collision_balls, (Ball){true, 0, 0, cp});
        sp.x -= .5 * D; sp.y += D;
    }
}

void init(Balls* collision_balls)
{
    SetTargetFPS(FPS);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Plinko");
    setBoard(collision_balls);
}

void deinit(Ball* alloc_collision_ball_mem)
{
    free(alloc_collision_ball_mem);
    CloseWindow();
}

int main()
{
    Balls collision_balls = {0, CAP, malloc(CAP)};
    init(&collision_balls);
    
    while(!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BLACK);
            drawBalls(&collision_balls);
        EndDrawing();
    }

    deinit(collision_balls.ball);
    return 0;
}