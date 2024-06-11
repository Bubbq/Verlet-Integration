#include "headers/raylib.h"
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
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
};

typedef struct
{
   float ball_bet;
   int bet_amount;
   float balance;
} Player;

const int RADIUS = 150;
const int SCREEN_SIZE = 700;

const float ADD_TIME = 3.0f;
const float GRAVITY = 1000.0f;

const int MIN_SPEED = 100;
const int MAX_SPEED = 500;
const int BALL_RADIUS = 10;

const int FPS = 60;
const float FRAME_TIME = (1.0f / FPS);

const Vector2 CENTER_POINT = {SCREEN_SIZE / 2.0f, SCREEN_SIZE / 2.0f};
const size_t INIT_CAP = (25 * sizeof(Ball));

const char* BET_TITLE = "BETTING INFO:";
const char* CURR_BALANCE = "-> BALANCE:";
const char* CURR_BET_AMOUNT = "-> BET AMOUNT:";
const char* BALL_BET = "-> NUMBER OF BALLS UNTIL EXIT";
const char* LOSE = "YOU LOST:";
const char* WIN = "YOU WIN:";

Player player;
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
    float r = GetRandomValue(1, (RADIUS - (BALL_RADIUS * 2.0f)));
    
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
	float angle = (atan2f((v1.y - v2.y), (v1.x - v2.x)) * RAD2DEG);
	
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
    all_balls.ball = malloc(INIT_CAP);
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
        Ball* ball2 = &all_balls.ball[i];
        // prevent colliding with itself
        if(Vector2Equals(ball->pos, ball2->pos)) continue;
        if(CheckCollisionCircles(ball->pos, BALL_RADIUS, ball2->pos, BALL_RADIUS)) collide(ball, ball2->pos, (BALL_RADIUS * 2.0f));
    }
}

void payout(bool win)
{   
    if(win) player.balance += (player.bet_amount * 2);
    else player.balance -= player.bet_amount;
}

void handleBorderCollision(Ball* ball, float start_angle, float end_angle)
{
    float ca = getAngle(ball->pos, CENTER_POINT);
    if (Vector2Distance(ball->pos, CENTER_POINT) + BALL_RADIUS >= RADIUS)
    {
        // game is over
        if((ca >= start_angle) && (ca <= end_angle))
        {
            game_state = DONE;
            payout((player.ball_bet == all_balls.size));
        }
        else collide(ball, CENTER_POINT, (RADIUS - BALL_RADIUS));
    }
}

void moveBall(Ball* ball, float start_angle, float end_angle)
{
    // ball velocity grows by GRAV every second
    ball->vy += (GRAVITY * FRAME_TIME);

    // ball moves by vx/vy pixels every second
    ball->pos.y += (ball->vy * FRAME_TIME);
    ball->pos.x += (ball->vx * FRAME_TIME);

    // ball collision
    handleBallCollision(ball);
    
    // ball hitting the circle's edges
    handleBorderCollision(ball, start_angle, end_angle);
}

void getRandomCircleSectorAngle(float* min, float* max)
{
    *min = GetRandomValue(200, 300);
    *max = *min + 45;
}

void placeBet()
{
    char balance[CHAR_LIMIT];
    char ball_bet[CHAR_LIMIT];
    char bet_amount[CHAR_LIMIT];

    sprintf(ball_bet, "%.0f", player.ball_bet);
    sprintf(balance, "%s $%.2f", CURR_BALANCE, player.balance);
    sprintf(bet_amount, "%s $%d", CURR_BET_AMOUNT, player.bet_amount);

    // displaying current betting information
    DrawText(BET_TITLE, 0, 55, 20, LIME);
    DrawText(BALL_BET, 0, 90, 15, LIME);
    DrawText(balance, 0, 75, 15, LIME);

    // choosing how many balls will it take to leave the circle
    const Rectangle SLIDER = {MeasureText(BALL_BET, 15) + 5, 90, 100, 12};
    GuiSliderBar(SLIDER, "", "", &player.ball_bet, 1, 100);
    DrawText(ball_bet, 365, 90, 15, LIME);

    // changing amount of money to bet with
    DrawText(bet_amount, 0, 105, 15, LIME);
    const Rectangle INC_BTN = {MeasureText(bet_amount, 15) + 10, 105, 15, 15};
    const Rectangle DEC_BTN = {MeasureText(bet_amount, 15) + 30, 105, 15, 15};
    if(GuiButton(INC_BTN, "+") && (player.bet_amount + 5 <= player.balance)) player.bet_amount += 5;
    if(GuiButton(DEC_BTN, "-") && (player.bet_amount - 5 > 0)) player.bet_amount -= 5;
}

void displayResult(bool win)
{
    // result dialouge
    char rd[CHAR_LIMIT];

    if(win) sprintf(rd, "%s $%d!", WIN, (player.bet_amount * 2));
    else sprintf(rd, "%s $%d!", LOSE, player.bet_amount);
    
    DrawText(rd, CENTER_POINT.x - (MeasureText(rd, 20) / 2.0f), CENTER_POINT.y, 20, (win ? GREEN : MAROON));
}

void init(float* start_angle, float* end_angle)
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Circle");
    
    all_balls.size = 0;
    all_balls.cap = INIT_CAP;
    all_balls.ball = malloc(INIT_CAP);
    
    game_state = READY;

    player.ball_bet = 0.0f;
    player.balance = 100.0f;
    player.bet_amount = 20.0f;

    getRandomCircleSectorAngle(start_angle, end_angle);
}

void deinit(Ball* alloc_mem)
{
    free(alloc_mem);
    CloseWindow();
}

int main()
{
    char ball_count_dialouge[CHAR_LIMIT];
    float start_angle;
    float end_angle;

    init(&start_angle, &end_angle);

    while(!WindowShouldClose())
    {
        int cidx = (all_balls.size == 0) ? 0 : all_balls.size - 1;
        Ball* current_ball = &all_balls.ball[cidx];

        sprintf(ball_count_dialouge, "BALL COUNT: %d", all_balls.size);

        BeginDrawing();
            ClearBackground(BLACK);

            DrawCircleLinesV(CENTER_POINT, RADIUS, LIME);
            DrawCircleSector(CENTER_POINT, RADIUS + 2, start_angle, end_angle, 10, BLACK);

            DrawText(ball_count_dialouge, 0, 15, 19, LIME);
            DrawFPS(0, 0);
            drawBalls();

            switch (game_state)
            {
                case ACTIVE:
                    if(TimerDone(add_timer))
                    {
                        addBall();
                        StartTimer(&add_timer, ADD_TIME);
                    }    
                    if(all_balls.size > 0) moveBall(current_ball, start_angle, end_angle);
                    break;
                case DONE:
                    displayResult((player.ball_bet == all_balls.size));
                    if(GuiButton((Rectangle){(SCREEN_SIZE / 2.0f) - 50, (SCREEN_SIZE - 150), 100, 50}, "OK")) 
                    {
                        resetBalls();
                        getRandomCircleSectorAngle(&start_angle, &end_angle);
                        add_timer.lifeTime = 0;
                        game_state = READY;
                    }
                    break;
                case READY: 
                    placeBet();
                    if(GuiButton((Rectangle){(SCREEN_SIZE / 2.0f) - 50, (SCREEN_SIZE - 150), 100, 50}, "START")) game_state = ACTIVE;
                    break;
                default:
                    break;
            }

        EndDrawing();
    }

    deinit(all_balls.ball);
    return 0;
}