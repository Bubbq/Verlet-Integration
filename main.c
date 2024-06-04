#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <string.h>
#define LIMIT 1024

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

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

enum Gamestate
{
   // objects are currently moving
   ACTIVE = 0,
   // the winner of the game is found
   DONE = 1,
   // game is ready to start
   READY = 2,
   // quiting the  game
   QUIT = 3,
};

enum Type
{
   UNDF = -1,
   ROCK = 0,
   PAPER = 1,
   SCISSORS = 2,
};

// represents one module in the game
typedef struct
{
   bool move;
   Vector2 pos;
   float dx, dy;
   enum Type type;
   Texture2D texture;
} Object;

typedef struct
{
   enum Type bet;
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
   DrawRectangleLinesEx(GA, 2, RAYWHITE);

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

void handleBorderCollision(Object* obj)
{
   // top & bottom of screen
   if((obj->pos.y < 0) || ((obj->pos.y + OBJ_SIZE) > GA.height))
       obj->dy *= -1;
  
   // left & right side of screen
   if((obj->pos.x < 0) || ((obj->pos.x + OBJ_SIZE) > GA.width))
       obj->dx *= -1;
}

void collisionWinner(Object* i, Object* j)
{
   switch (i->type)
   {
       // loses against paper and wins vs scissors
       case ROCK:
           if(j->type == PAPER)
               i->type = PAPER;
           else if(j->type == SCISSORS)
               j->type = ROCK;
           break;
      
       // loses against scissors and wins against rock
       case PAPER:
           if(j->type == SCISSORS)
               i->type = SCISSORS;
           else if(j->type == ROCK)
               j->type = PAPER;
           break;
      
       // loses against and wins vs. paper
       case SCISSORS:
           if(j->type == ROCK)
               i->type = ROCK;
           else if(j->type == PAPER)
               j->type = SCISSORS;
           break;

       default:
           break;
   }
}

void handleObjectCollision(Object* i, Object* j)
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
       for(int j = 0; j < OBJ_COUNT; j++)
       {
           // only collide with objects of different type
           if(objects[i].type == objects[j].type)
               continue;

           Rectangle potential_col = {objects[j].pos.x, objects[j].pos.y, OBJ_SIZE, OBJ_SIZE};

           if(CheckCollisionRecs(obj_area, potential_col))
               handleObjectCollision(&objects[i], &objects[j]);
       }
   }
}

void setObjects(Object objects[OBJ_COUNT], enum Gamestate* gs)
{
   *gs = READY;

   for(int i = 0; i < OBJ_COUNT; i++)
   {
       enum Type t = UNDF;

       // 10 objects of each type
       if(i < 10)
           t = ROCK;
       else if(i < 20)
           t = PAPER;
       else
           t = SCISSORS;

       objects[i].type = t;

       // randomize direction and position
       objects[i].dx = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
       objects[i].dy = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
       objects[i].pos = (Vector2){GetRandomValue(0, (GA.width - OBJ_SIZE)), GetRandomValue(0, (GA.height - OBJ_SIZE))};
   }
}

enum Type checkWinner(Object objects[OBJ_COUNT])
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
               if(rcnt == OBJ_COUNT)
                   return ROCK;
           break;
          
           case PAPER: pcnt++;
               if(pcnt == OBJ_COUNT)
                   return PAPER;
           break;
          
           case SCISSORS:
               scnt++;
               if(scnt == OBJ_COUNT)
                   return SCISSORS;
           break;
          
           default:  
               break;
       }
   }

   return UNDF;
}

bool broke(float balance)
{
   return (balance == 0);
}

void betInfo(Player* player, char btnd[LIMIT], char cbd[LIMIT])
{
   char balance_dialouge[LIMIT];
   char bet_amount_dialouge[LIMIT];
   strcpy(btnd, "START");

   // choosing bet
   if(GuiButton((Rectangle){0, GA.height + 5, 20, 20}, "R"))
   {
       player->bet = ROCK;
       sprintf(cbd, "%sROCK", CURR_BET);
   }

   if(GuiButton((Rectangle){30, GA.height + 5, 20, 20}, "P"))
   {
       player->bet = PAPER;
       sprintf(cbd, "%sPAPER", CURR_BET);
   }

   if(GuiButton((Rectangle){60, GA.height + 5, 20, 20}, "S"))
   {
       player->bet = SCISSORS;
       sprintf(cbd, "%sSCISSORS", CURR_BET);
   }
  
   // displaying current betting information
   sprintf(balance_dialouge, "BALANCE: %.2f", player->balance);
   sprintf(bet_amount_dialouge, "BET AMOUNT: %.2f", player->bet_amount);
   DrawText(balance_dialouge, 0, GA.height + 35, 10, RAYWHITE);
   DrawText(bet_amount_dialouge, 0, GA.height + 50, 10, RAYWHITE);

   // choosing bet amount
   if(!broke(player->balance))
       GuiSliderBar((Rectangle){0, GA.height + 65, 100, 20}, "", "", &player->bet_amount, 0.10, player->balance);
}

void gameResult(Player* player, enum Gamestate* gs, char btnd[LIMIT], char rd[LIMIT], bool win)
{
   // game payout and dialouge
   if(win)
   {
       player->balance += (player->bet_amount * 2);
       strcpy(rd, WIN);
   }
  
   else
   {
       player->balance -= player->bet_amount;
       strcpy(rd, LOSE);
   }
  
   // updating game and player information
   *gs = DONE;
   player->bet = UNDF;
   player->bet_amount = (player->balance * 0.20f);
   strcpy(btnd, "RESET");
}

void init(Object objects[OBJ_COUNT], Player* player, enum Gamestate* gs)
{
   player->bet = UNDF;
   player->balance = 100.00;
   player->bet_amount = 20.00;
  
   SetTargetFPS(FPS);
   setObjects(objects, gs);
   SetTraceLogLevel(LOG_ERROR);
   InitWindow(SCRA.width, SCRA.height, "Rock, Paper, Scissors Sim");
}

int main()
{
   // button dialouge
   char btnd[LIMIT] = "START";
   // current bet dialouge
   char cbd[LIMIT];
   // result dialouge
   char rd[LIMIT];
   Player player;
   Object objects[OBJ_COUNT];
   Timer text_timer;
   enum Gamestate gs = READY;

   init(objects, &player, &gs);

   while((gs != QUIT) && !WindowShouldClose())
   {
       // the winning object type
       enum Type wobjt = checkWinner(objects);
       if(broke(player.balance))
           strcpy(btnd, "QUIT");

       BeginDrawing();
           ClearBackground(BLACK);
           drawObjects(objects);
          
           switch (gs)
           {
               case ACTIVE:
                   // moves objects on screen
                   updateObjects(objects);
                   // show current bet placed
                   DrawText(cbd, (SCRA.width / 2.0f) - (MeasureText(cbd, 30) / 2.0f), GA.height + 40, 30, RAYWHITE);
                   // update player's balance when game ends
                   if(wobjt != UNDF) 
                       gameResult(&player, &gs, btnd, rd, (wobjt == player.bet));
                   break;

               case DONE:
                   // display result of the game or inform of invalid balance
                   if(!broke(player.balance))
                       DrawText(rd, (SCRA.width / 2.0f) - (MeasureText(rd, 30) / 2.0), (SCRA.height / 2.0f), 30, RAYWHITE);
                   else
                       DrawText(NO_MONEY, (SCRA.width / 2.0f) - (MeasureText(NO_MONEY, 30) / 2.0f), (SCRA.height / 2.0f), 30, RAYWHITE);
                   // either reset the board or quit bc of no money
                   if(GuiButton((Rectangle){(SCRA.width / 2.0f) - 100, GA.height + ((SCRA.height - GA.height) / 2.0f) - 25, 200, 30}, btnd))
                   {
                       if(strcmp(btnd, "RESET") == 0)
                           setObjects(objects, &gs);
                       else if(strcmp(btnd, "QUIT") == 0)
                           gs = QUIT;
                   }
                   break;

               case READY:
                   //  player chooses object type to win with some amount of money
                   betInfo(&player, btnd, cbd);
                   // btn that starts the game when valid bet placed
                   if(GuiButton((Rectangle){(SCRA.width / 2.0f) - 100, GA.height + ((SCRA.height - GA.height) / 2.0f) - 25, 200, 30}, btnd))
                   {
                       if(player.bet != UNDF)
                           gs = ACTIVE;
                       else
                           StartTimer(&text_timer, TEXT_TIME);
                   }
                   // reminds player to place a bet if they tried starting the game without placing one
                   if(!TimerDone(text_timer))
                       DrawText(NO_BET, (SCRA.width / 2.0f) - (MeasureText(NO_BET, 30) / 2.0f), (GA.height / 2.0f), 30, RAYWHITE);
                   break;

               default:
                   break;
           }
      
       EndDrawing();
   }

   CloseWindow();
   return 0;
}