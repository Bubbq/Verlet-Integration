#include <stdio.h>
#include <stdlib.h>
#include "headers/raylib.h"
#include <raymath.h>

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
    Color c;
    Vector2 pos;
    float bet;
    bool active;
    float vx, vy;
} Ball;

typedef struct
{
    Rectangle border;
    float multp;
} PayoutBox;

typedef struct
{
    int size;
    size_t cap;
    Ball* ball;
} Balls;

typedef struct
{
    PayoutBox* box;
    size_t cap;
    int size;
} Boxes;

const float ADD_TIME = 0.25f;

const int SCREEN_SIZE = 1000;

const int LEVELS = 14;
const int TOP_LEVEL = 3;
const int BALL_DIST = 48;
const int RADIUS_BOARD = 8;
const int RADIUS_GAME = 11;
const float GRAVITY = 700.0f;

const int FPS = 60;
const float FRAME_TIME = 1.0f / FPS;

// y level of payout boxes
const int Y_POS = 100 + ((LEVELS - 1) * BALL_DIST) + (RADIUS_BOARD * 2.0f);

void StartTimer(Timer *timer, double lifetime)
{
   timer->startTime = GetTime();
   timer->lifeTime = lifetime;
}

bool TimerDone(Timer timer) {return GetTime() - timer.startTime >= timer.lifeTime;}

// returns the angle between 2 cartesian coords
float getAngle(Vector2 v1, Vector2 v2)
{
	float angle = (atan2f((v1.y - v2.y), (v1.x - v2.x)) * RAD2DEG);

    if(angle > 360) angle -= 360;
    else if(angle < 0) angle += 360;
	
    return angle;
}

// returns a cartesian coord thats in-between the 3 balls on the first level
Vector2 getRandomBallPosition() {return (Vector2){GetRandomValue(((SCREEN_SIZE / 2.0f) - BALL_DIST) + RADIUS_GAME,  ((SCREEN_SIZE / 2.0f) + BALL_DIST) - RADIUS_GAME), 50};}

// realloc mem for dynamic lists
void resizeBoxes(Boxes* boxes)
{
    boxes->cap *= 2;
    boxes->box = realloc(boxes->box, boxes->cap);
}

void resizeBalls(Balls* balls)
{
    balls->cap *= 2;
    balls->ball = realloc(balls->ball, balls->cap);
}

// adds element to dynamic lists
void addBox(Boxes* boxes, PayoutBox new_box)
{
    if(boxes->size * sizeof(PayoutBox) == boxes->cap) resizeBoxes(boxes);
    boxes->box[boxes->size++] = new_box;
}

void addBall(Balls* balls, Ball new_ball)
{
    if(balls->size * sizeof(Ball) == balls->cap) resizeBalls(balls);
    balls->ball[balls->size++] = new_ball;
}

// drawing game elements
void drawBalls(Balls* balls, const int RADIUS) 
{
    for(int i = 0; i < balls->size; i++) if(balls->ball[i].active) DrawCircleV(balls->ball[i].pos, RADIUS, balls->ball[i].c);
}

void drawPayoutBoxes(Boxes* boxes)
{
    char multiplier_text[CHAR_LIMIT];

    for(int i = 0; i < boxes->size; i++)
    {
        PayoutBox* box = &boxes->box[i];
        DrawRectangleRec(box->border, RAYWHITE);
        sprintf(multiplier_text, "%.2f", box->multp);
        DrawText(multiplier_text, box->border.x + 3, (box->border.y + (box->border.height / 2.0f)), 15, BLACK);
    }
}

// uses vector reflection formula, v' = v -2(v * n)n, to handle collision between a ball at some point
void collide(Ball* ball, Vector2 collision_pos, float radius)
{
    // collision angle
    float ca = getAngle(ball->pos, collision_pos);
    
    // prevents ball from standing still at top of board ball
    if(ca == 270) ca++;

    // the unit vector of collision 
    Vector2 n = {cosf(ca * DEG2RAD), sinf(ca * DEG2RAD)};

    // (v * n), the dot product between the current velocity and the normal vector
    float dp = Vector2DotProduct((Vector2){ball->vx, ball->vy}, n);

    // calculating v'
    ball->vx -= (1.2f * dp * n.x);
    ball->vy -= (1.2f * dp * n.y);

    // position correction
    ball->pos.x = collision_pos.x + (radius * n.x);
    ball->pos.y = collision_pos.y + (radius * n.y);
}

// determining if player can make that bet
bool broke(float balance, float bet) {return (balance - bet < 0);}

// paying out player based on the multiplier the ball hits
void payout(float* balance, float bet, float multiplier) {*balance += (bet * multiplier);}

// game ball collision handling
void handleBallCollision(Ball* ball, Balls* board_balls)
{
    for(int i = 0; i < board_balls->size; i++) 
    {
        Ball* board_ball = &board_balls->ball[i];
        if(CheckCollisionCircles(ball->pos, RADIUS_GAME, board_ball->pos, RADIUS_BOARD)) collide(ball, board_ball->pos, (RADIUS_BOARD + RADIUS_GAME));
    }
}

void handlePayoutBoxCollision(Boxes* boxes, Ball* ball, float* balance)
{
    for(int i = 0; i < boxes->size; i++)
    {
        PayoutBox* pb = &boxes->box[i];

        // payout player when active ball hits a payout box
        if(CheckCollisionCircleRec(ball->pos, RADIUS_GAME, pb->border) && ball->active)
        {
            payout(balance, ball->bet, pb->multp);
            ball->active = false;
        }
    }
}

// where player can choose how much each ball is worth
void editBet(float* bet, float balance)
{
    char bet_amount[CHAR_LIMIT];
    
    sprintf(bet_amount, "$%.2f", *bet);
    DrawText("BALL AMOUNT", 0, 30, 19, LIME);
    GuiSliderBar((Rectangle){MeasureText("BALL AMOUNT", 19) + 5, 33, 100, 12}, "", bet_amount, bet, 0.0f, balance);
}

// holding down 'PLAY' btn places bet
void placeBet(Balls* balls, Timer* timer, float* balance, float bet)
{
    Rectangle border = (Rectangle){(SCREEN_SIZE / 2.0f) - 50, Y_POS + 100, 100, 30};

    DrawRectangleRec(border, RED);
    DrawText("PLAY", border.x + (border.width / 2.0f) - (MeasureText("PLAY", 20) / 2.0f), border.y + (border.height / 2.0f) - 10, 20, YELLOW);
    
    if((IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), border)) && (TimerDone(*timer)) && (!broke(*balance, bet)))
    {
        *balance -= bet;
        StartTimer(timer, ADD_TIME);
        addBall(balls, (Ball){RED, getRandomBallPosition(), bet, true, 0, 0});
    }
}

// shows how much money the player has in account
void displayBalance(float balance)
{
    char text[CHAR_LIMIT];
    sprintf(text, "BALANCE: $%.2f", balance);
    DrawText(text, 0, 15, 19, LIME);
}

// updates the position of every active game ball
void moveBalls(Balls* balls, Balls* board_balls, Boxes* boxes, float* balance)
{
    for(int i = 0; i < balls->size; i++)
    {
        Ball* ball = &balls->ball[i];
        
        // only updating active balls
        if(!ball->active) continue;

        // inc falling down speed
        ball->vy += GRAVITY * FRAME_TIME;

        // ball moves vx, vy pxs every second
        ball->pos.x += ball->vx * FRAME_TIME;
        ball->pos.y += ball->vy * FRAME_TIME;

        // hitting board ball(s)
        handleBallCollision(ball, board_balls);

        // hitting payout box(s)
        handlePayoutBoxCollision(boxes, ball, balance);
    }
}

// initialization of game objects
void setPayoutBoxes(Boxes* boxes, Vector2 last_row, int box_count)
{
    // rare edge multipliers
    float multiplier = LEVELS;
    
    // payout box dimensions
    int width = BALL_DIST - (RADIUS_BOARD * 2.0f);
    int height = width;
    
    // create box with higher multiplers on the outside and lower inside, position is based on the last row of board balls
    for(int i = 0; i < box_count; i++, last_row.x += BALL_DIST, multiplier = (i <= box_count / 2) ? multiplier / 2 :  multiplier * 2) addBox(boxes, (PayoutBox){(Rectangle){(last_row.x + RADIUS_BOARD), Y_POS, width, height}, multiplier});
}

void setBoard(Balls* board_balls, Boxes* boxes)
{
    // position of first ball in LEVEL levels 
    Vector2 starting_row_pos = {(SCREEN_SIZE / 2.0f) - BALL_DIST, 100};

    for(int i = 0; i < LEVELS; i++)
    {
        Vector2 current_pos = starting_row_pos;
        
        // create ball at current_pos and move right BALL_DIST apart
        for(int j = 0; j < (TOP_LEVEL + i); j++, current_pos.x += BALL_DIST) 
        {
            addBall(board_balls, (Ball){RAYWHITE, current_pos, 0, true, 0, 0});
            
            // create payout boxes on the last row
            if(i == (LEVELS - 1)) setPayoutBoxes(boxes, starting_row_pos, (TOP_LEVEL + i - 1));
        }

        starting_row_pos.x -= .5 * BALL_DIST; starting_row_pos.y += BALL_DIST;
    }
}

// instantiantion and termination of program
void init(Balls* board_balls, Boxes* boxes)
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Plinko");
    setBoard(board_balls, boxes);
}

void deinit(Ball* alloc_collision_ball_mem, Ball* alloc_game_ball_mem, PayoutBox* alloc_box_mem)
{
    free(alloc_collision_ball_mem);
    free(alloc_game_ball_mem);
    free(alloc_box_mem);
    CloseWindow();
}

int main()
{
    Timer add_timer;

    Balls game_balls = {0, sizeof(Ball), malloc(sizeof(Ball))};
    Balls board_balls = {0, sizeof(Ball), malloc(sizeof(Ball))};
    Boxes boxes = {malloc(sizeof(PayoutBox)), sizeof(PayoutBox), 0};

    float balance = 100.0f;
    float bet = 1.0f;

    init(&board_balls, &boxes);
    while(!WindowShouldClose())
    {
        moveBalls(&game_balls, &board_balls, &boxes, &balance);
        BeginDrawing();
            ClearBackground(BLACK);
            displayBalance(balance);
            DrawFPS(0, 0);
            editBet(&bet, balance);
            placeBet(&game_balls, &add_timer, &balance, bet);
            drawBalls(&game_balls, RADIUS_GAME);
            drawBalls(&board_balls, RADIUS_BOARD);
            drawPayoutBoxes(&boxes);
        EndDrawing();
    }
    deinit(board_balls.ball, game_balls.ball, boxes.box);
    return 0;
}