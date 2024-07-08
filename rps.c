#include "headers/raylib.h"
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LIMIT 1024

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

const int FPS = 60;
const float TEXT_TIME = 0.50;

const char* NO_BET = "PLACE A BET TO START!";
const char* NO_MONEY = "NO MONEY TO PLAY!";
const char* CURR_BET = "CURRENT BET: ";
const char* LOSE = "YOU LOSE!";
const char* WIN = "YOU WIN!";

// screen & game area
const Rectangle SCRA = {0, 0, 500, 600};
const Rectangle GA = {0,0, 500, 500};

const int OBJ_SIZE = 16;
const int OBJ_COUNT = 30;
const float OBJ_SPEED = 3.00;

typedef enum 
{
   // objects are currently moving
   ACTIVE = 0,
   // the winner of the game is found
   DONE = 1,
   // game is ready to start
   READY = 2,
   // quiting the  game
   QUIT = 3,
} Gamestate;

typedef enum
{
   UNDF = -1,
   ROCK = 0,
   PAPER = 1,
   SCISSORS = 2,
} Type;

// represents one module in the game
typedef struct
{
   bool move;
   Vector2 pos;
   float dx, dy;
   Type type;
} Object;

typedef struct
{
   Type bet;
   float balance;
   float bet_amount;
} Player;

typedef struct
{
   double startTime;
   double lifeTime;
} Timer;

void StartTimer(Timer *timer, double lifetime)
{
   timer->startTime = GetTime();
   timer->lifeTime = lifetime;
}

bool TimerDone(Timer timer)
{
   return GetTime() - timer.startTime >= timer.lifeTime;
}

void drawObjects(Object objects[OBJ_COUNT])
{
   for(int i = 0; i < OBJ_COUNT; i++)
   {
       Color c;
       switch (objects[i].type)
       {
           case ROCK: c = RED; break;
           case PAPER: c = YELLOW; break;
           case SCISSORS: c = DARKGREEN; break;
           default:
               break;
       }

       DrawRectangle(objects[i].pos.x, objects[i].pos.y, OBJ_SIZE, OBJ_SIZE, c);
   }
}

void collisionWinner(Object* i, Object* j)
{
   switch (i->type)
   {
       case ROCK:
           if(j->type == PAPER) i->type = PAPER;
           else if(j->type == SCISSORS) j->type = ROCK;
           break;
       case PAPER:
           if(j->type == SCISSORS) i->type = SCISSORS;
           else if(j->type == ROCK) j->type = PAPER;
           break;
       case SCISSORS:
           if(j->type == ROCK) i->type = ROCK;
           else if(j->type == PAPER) j->type = SCISSORS;
           break;
       default:
           break;
   }
}

void collide(Object* i, Object* j)
{
   // Velocities before collision
   Vector2 vi = {i->dx, i->dy};
   Vector2 vj = {j->dx, j->dy};

   // finds the position of obj i w.r.t obj j, then, scales this resulting vector downw to magnitude of 1
   Vector2 normal = Vector2Normalize(Vector2Subtract(i->pos, j->pos));

   // tangential vector, role is to b perpendicular to the normal
   Vector2 tangent = (Vector2){-normal.y, normal.x};

   // scale these vectors by how fast the objects i and j were going before their collision
   float vi_normal = Vector2DotProduct(vi, normal);
   float vj_normal = Vector2DotProduct(vj, normal);
   float vi_tangent = Vector2DotProduct(vi, tangent);
   float vj_tangent = Vector2DotProduct(vj, tangent);

   // collision velocitites
   Vector2 vi_new = Vector2Add(Vector2Scale(normal, vj_normal), Vector2Scale(tangent, vi_tangent));
   Vector2 vj_new = Vector2Add(Vector2Scale(normal, vi_normal), Vector2Scale(tangent, vj_tangent));

   // updating object velocities
   i->dx = vi_new.x; i->dy = vi_new.y;
   j->dx = vj_new.x; j->dy = vj_new.y;

   // gets winner from collision
   collisionWinner(i, j);
}

void handleObjectCollision(Object* i, Object objects[OBJ_COUNT], Rectangle area)
{
    for(int j = 0; j < OBJ_COUNT; j++)
    {
        // only collide with objects of different type
        if(i->type == objects[j].type) continue;

        Rectangle potential_col_area = {objects[j].pos.x, objects[j].pos.y, OBJ_SIZE, OBJ_SIZE};

        if(CheckCollisionRecs(area, potential_col_area)) collide(i, &objects[j]);
    }
}

void handleBorderCollision(Object* obj)
{
   // top & bottom of screen
   if((obj->pos.y < 0) || ((obj->pos.y + OBJ_SIZE) > GA.height))
       obj->dy *= -1;
  
   // left & right side of screen
   if((obj->pos.x < 0) || ((obj->pos.x + OBJ_SIZE) > GA.width))
       obj->dx *= -1;
}

void updateObjects(Object objects[OBJ_COUNT])
{
   for(int i = 0; i < OBJ_COUNT; i++)
   {
       // Move by component magnitude
       objects[i].pos = Vector2Add(objects[i].pos, (Vector2){objects[i].dx, objects[i].dy});

       // reflecting objects when they hit screen borders
       handleBorderCollision(&objects[i]);

       // Area of ith object
       Rectangle obj_area = {objects[i].pos.x, objects[i].pos.y, OBJ_SIZE, OBJ_SIZE};

       // handling collisions with other objects
       handleObjectCollision(&objects[i], objects, obj_area);
   }
}

void setObjects(Object objects[OBJ_COUNT])
{
   for(int i = 0; i < OBJ_COUNT; i++)
    {
        Type t = UNDF;

        // 10 rock, paper, and scissor objects
        if(i < 10) t = ROCK;
        else if(i < 20) t = PAPER;
        else t = SCISSORS;

        objects[i].type = t;

        // randomize direction and position
        objects[i].dx = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
        objects[i].dy = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
        objects[i].pos = (Vector2){GetRandomValue(0, (GA.width - OBJ_SIZE)), GetRandomValue(0, (GA.height - OBJ_SIZE))};
   }
}

Type checkWinner(Object objects[OBJ_COUNT])
{
   int rcnt = 0;
   int pcnt = 0;
   int scnt = 0;

   for(int i = 0; i < OBJ_COUNT; i++)
   {
       switch (objects[i].type)
       {
           case ROCK:
               rcnt++;
               if(rcnt == OBJ_COUNT) return ROCK;
           break;
           case PAPER: pcnt++;
               if(pcnt == OBJ_COUNT) return PAPER;
           break;
           case SCISSORS:
               scnt++;
               if(scnt == OBJ_COUNT) return SCISSORS;
           break;
           default:  
               break;
       }
   }

   return UNDF;
}

bool broke(float balance) { return (balance == 0); }

void placeBet(Player* player)
{
   char balance_dialouge[LIMIT];
   char bet_amount_dialouge[LIMIT];

   // choosing which object type will win
   if(GuiButton((Rectangle){0, GA.height + 5, 20, 20}, "R")) player->bet = ROCK;
   if(GuiButton((Rectangle){30, GA.height + 5, 20, 20}, "P")) player->bet = PAPER;
   if(GuiButton((Rectangle){60, GA.height + 5, 20, 20}, "S")) player->bet = SCISSORS;
  
   // displaying current betting information
   sprintf(balance_dialouge, "BALANCE: %.2f", player->balance);
   sprintf(bet_amount_dialouge, "BET AMOUNT: %.2f", player->bet_amount);
   DrawText(balance_dialouge, 0, GA.height + 35, 10, RAYWHITE);
   DrawText(bet_amount_dialouge, 0, GA.height + 50, 10, RAYWHITE);

   // choosing bet amount
   if(!broke(player->balance)) GuiSliderBar((Rectangle){0, GA.height + 65, 100, 20}, "", "", &player->bet_amount, 0.10, player->balance);
}

void gameResult(Player* player, Gamestate* gs, bool win)
{
    // payout
    if(win) player->balance += (player->bet_amount * 3);
    else player->balance -= player->bet_amount;
    
    // restart betting information
    player->bet = UNDF;
    player->bet_amount = (player->balance * 0.20f);
}

void displayResult(bool win)
{
    char text[LIMIT];
    
    if(win) sprintf(text, "%s", WIN);
    else sprintf(text, "%s", LOSE);

    DrawText(text, (SCRA.width / 2.0f) - (MeasureText(text, 30) / 2.0), (SCRA.height / 2.0f), 30, RAYWHITE);
}

void displayCurrentBet(Type bet)
{
    char text[LIMIT];
    char bet_text[LIMIT];

    switch (bet)
    {
        case ROCK: strcpy(bet_text, "ROCK"); break;
        case PAPER: strcpy(bet_text, "PAPER"); break;
        case SCISSORS: strcpy(bet_text, "SCISSORS"); break;
        default:    
            break;
    }

    sprintf(text, "%s%s", CURR_BET, bet_text);
    DrawText(text, (SCRA.width / 2.0f) - (MeasureText(text, 30) / 2.0f), GA.height + 40, 30, RAYWHITE);
}

void init(Object objects[OBJ_COUNT], Player* player, Gamestate* gs)
{
    // player betting information
    player->bet = UNDF;
    player->balance = 100.00;
    player->bet_amount = 20.00;

    *gs = READY;

    SetTargetFPS(FPS);
    setObjects(objects);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCRA.width, SCRA.height, "Rock, Paper, Scissors Sim");
}

int main()
{
   Player player;
   Timer text_timer;
   Object objects[OBJ_COUNT];
   Gamestate game_state = READY;

   init(objects, &player, &game_state);

   while((game_state != QUIT) && !WindowShouldClose())
   {
       // the winning object type
       Type wobjt = checkWinner(objects);

       BeginDrawing();
           ClearBackground(BLACK);
           DrawRectangleLinesEx(GA, 2, RAYWHITE);
           drawObjects(objects);
           switch (game_state)
           {
               case ACTIVE:
                   updateObjects(objects);
                   displayCurrentBet(player.bet);
                   if(wobjt != UNDF) 
                   {
                        game_state = DONE;
                        gameResult(&player, &game_state, (wobjt == player.bet));
                   }
                   break;
               case DONE:
                   if(!broke(player.balance)) displayResult(wobjt == player.bet);
                   else DrawText(NO_MONEY, (SCRA.width / 2.0f) - (MeasureText(NO_MONEY, 30) / 2.0f), (SCRA.height / 2.0f), 30, RAYWHITE);
                   if(GuiButton((Rectangle){(SCRA.width / 2.0f) - 100, GA.height + ((SCRA.height - GA.height) / 2.0f) - 25, 200, 30}, broke(player.balance) ? "QUIT" : "RESET"))
                   {
                       if(!broke(player.balance))
                       {
                            game_state = READY;
                            setObjects(objects);
                       }
                       else game_state = QUIT;
                   }
                   break;
               case READY:
                   placeBet(&player);
                   if(GuiButton((Rectangle){(SCRA.width / 2.0f) - 100, GA.height + ((SCRA.height - GA.height) / 2.0f) - 25, 200, 30}, "START"))
                   {
                       if(player.bet != UNDF) game_state = ACTIVE;
                       else StartTimer(&text_timer, TEXT_TIME);
                   }
                   if(!TimerDone(text_timer)) DrawText(NO_BET, (SCRA.width / 2.0f) - (MeasureText(NO_BET, 30) / 2.0f), (GA.height / 2.0f), 30, RAYWHITE);
                   break;
               default:
                   break;
           }
       EndDrawing();
   }
   CloseWindow();
   return 0;
}