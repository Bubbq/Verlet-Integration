#include <stdio.h>
#include <stdlib.h>
#include "headers/raylib.h"
#include <raymath.h>

typedef enum
{
    BOARD = 0,
    GAME = 1,
} Type;

typedef struct
{
    Color c;
    Type type;
    Vector2 pos;
    float vx, vy;
} Ball;

typedef struct
{
    int size;
    size_t cap;
    Ball* ball;
} Balls;

const int FPS = 60;
const int RADIUS = 6;
const int LEVELS = 10;
const int BALL_DIST = 45;
const int SCREEN_SIZE = 700;
const float GRAVITY = 700.0f;
const float FRAME_TIME = 1.0f / FPS;
const size_t CAP = sizeof(Ball) * 75;

float getAngle(Vector2 v1, Vector2 v2)
{
	float angle = (atan2f((v1.y - v2.y), (v1.x - v2.x)) * RAD2DEG);

    if(angle > 360) angle -= 360;
    else if(angle < 0) angle += 360;
	
    return angle;
}

Vector2 getRandomBallPosition()
{
    const float HALF = SCREEN_SIZE / 2.0f;
    return (Vector2){GetRandomValue((HALF - BALL_DIST) + RADIUS,  (HALF + BALL_DIST) - RADIUS), 50};
}

void resizeBalls(Balls* balls)
{
    balls->cap *= 2;
    balls->ball = realloc(balls->ball, balls->cap);
}

void addBall(Balls* balls, Ball new_ball)
{
    if(balls->size * sizeof(Ball) == balls->cap) resizeBalls(balls);
    balls->ball[balls->size++] = new_ball;
}

bool inBounds(Vector2 ball_pos)
{
    return ((ball_pos.y + RADIUS) <= SCREEN_SIZE);
}

void drawBalls(Balls* balls) 
{
    for(int i = 0; i < balls->size; i++) if(inBounds(balls->ball[i].pos)) DrawCircleV(balls->ball[i].pos, RADIUS, balls->ball[i].c);
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
    ball->vx -= (1.0f * dp * n.x);
    ball->vy -= (1.0f * dp * n.y);

    // position correction
    ball->pos.x = collision_pos.x + (radius * n.x);
    ball->pos.y = collision_pos.y + (radius * n.y);
}

void handleBallCollision(Ball* ball, Balls* collision_balls)
{
    for(int i = 0; i < collision_balls->size; i++) if(CheckCollisionCircles(ball->pos, RADIUS, collision_balls->ball[i].pos, RADIUS)) collide(ball, collision_balls->ball[i].pos, (RADIUS * 2.0f));
}

void moveBalls(Balls* balls, Balls* collision_balls)
{
    for(int i = 0; i < balls->size; i++)
    {
        Ball* ball = &balls->ball[i];
        if(!inBounds(ball->pos)) continue;

        // inc falling down speed
        ball->vy += GRAVITY * FRAME_TIME;

        // ball moves vx, vy pxs every second
        ball->pos.x += ball->vx * FRAME_TIME;
        ball->pos.y += ball->vy * FRAME_TIME;

        // hitting board ball(s)
        handleBallCollision(ball, collision_balls);
    }
}

void setBoard(Balls* collision_balls)
{
    Vector2 starting_row_pos = {(SCREEN_SIZE / 2.0f) - BALL_DIST, 100};

    for(int i = 0; i < LEVELS; i++)
    {
        Vector2 current_pos = starting_row_pos;
        for(int j = 0; j < (3 + i); j++, current_pos.x += BALL_DIST) addBall(collision_balls, (Ball){RAYWHITE, BOARD, current_pos, 0, 0});
        starting_row_pos.x -= .5 * BALL_DIST; starting_row_pos.y += BALL_DIST;
    }
}

void init(Balls* collision_balls)
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Plinko");
    setBoard(collision_balls);
}

void deinit(Ball* alloc_collision_ball_mem, Ball* alloc_game_ball_mem)
{
    free(alloc_collision_ball_mem);
    free(alloc_game_ball_mem);
    CloseWindow();
}

int main()
{
    Balls collision_balls = {0, CAP, malloc(CAP)};
    Balls game_balls = {0, CAP, malloc(CAP)};
    init(&collision_balls);
    while(!WindowShouldClose())
    {
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) addBall(&game_balls, (Ball){RED, GAME, getRandomBallPosition(), 0, 0});
        moveBalls(&game_balls, &collision_balls);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawFPS(0, 0);
            drawBalls(&game_balls);
            drawBalls(&collision_balls);
        EndDrawing();
    }
    deinit(collision_balls.ball, game_balls.ball);
    return 0;
}