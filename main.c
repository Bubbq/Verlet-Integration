#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <string.h>
#define LIMIT 1024

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

const int FPS = 60;
const int GAME_AREA = 500;
const float MIN_BALANCE = 0.10;
const float TEXT_TIME = 0.50;

const char* NO_BET = "PLACE A BET TO START!";
const char* NO_MONEY = "NO MONEY TO PLAY!";
const char* CURR_BET = "CURRENT BET: ";
const char* WIN = "YOU WIN!";
const char* LOSE = "YOU LOSE!";

// screen & game area
const Rectangle SCRA = {0, 0, 500, 600};
const Rectangle GA = {0,0, GAME_AREA, GAME_AREA};

const int OBJ_SIZE = 16;
const int OBJ_COUNT = 30;
const float OBJ_SPEED = 3.00;

// rock, paper, and scissor spawn areas
const Rectangle RA = {0, 0, (GAME_AREA / 2.0f), (GAME_AREA / 2.0f)};
const Rectangle PA = {(GAME_AREA  / 2.0f), 0, (GAME_AREA / 2.0f), (GAME_AREA / 2.0f)};
const Rectangle SA = {(GAME_AREA  / 4.0f), (GAME_AREA / 2.0f), (GAME_AREA / 2.0f), (GAME_AREA / 2.0f)};

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

// draws all objects on the screen
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

// collision with screen border
void handleBorderCollision(Object* obj)
{
	// top & bottom of screen
	if((obj->pos.y < 0) || ((obj->pos.y + OBJ_SIZE) > GAME_AREA)) obj->dy *= -1;
	// left & right side of screen
	if((obj->pos.x < 0) || ((obj->pos.x + OBJ_SIZE) > GAME_AREA)) obj->dx *= -1;
}

// determines the winner between obj i and j based on rock paper scissors rules
void collisionWinner(Object* i, Object* j)
{
	switch (i->type)
	{
		case ROCK: // loses against paper and wins vs scissors
			if(j->type == PAPER) i->type = PAPER; 
			else if(j->type == SCISSORS) j->type = ROCK;
			break;
			
		case PAPER: // loses against scissors and wins against rock
			if(j->type == SCISSORS) i->type = SCISSORS; 
			else if(j->type == ROCK) j->type = PAPER; 
			break;

		case SCISSORS: // loses against and wins vs. paper
			if(j->type == ROCK) i->type = ROCK; 
			else if(j->type == PAPER) j->type = SCISSORS;
			break;

		default:
			break;
	}
}

// collision from obj i with obj j
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

// to move each object within the screen
void updateObjects(Object objects[OBJ_COUNT])
{
	for(int i = 0; i < OBJ_COUNT; i++)
	{
		// Move by component magnitude
		objects[i].pos = Vector2Add(objects[i].pos, (Vector2){objects[i].dx, objects[i].dy});

		// Screen collisions
		handleBorderCollision(&objects[i]);

		// Area of ith object
		Rectangle obj_area = {objects[i].pos.x, objects[i].pos.y, OBJ_SIZE, OBJ_SIZE};

		// Collisions with other objects
		for(int j = 0; j < OBJ_COUNT; j++)
		{
			// only colliding with objects of different type
			if(objects[i].type == objects[j].type) continue;

			Rectangle potential_col = {objects[j].pos.x, objects[j].pos.y, OBJ_SIZE, OBJ_SIZE};

			if(CheckCollisionRecs(obj_area, potential_col)) handleObjectCollision(&objects[i], &objects[j]);
		}
	}
}

// adds rock, paper, and scissor objects to the area
void setObjects(Object objects[OBJ_COUNT])
{
	int i = 0;
	for(; i < 10; i++) {objects[i].type = ROCK; objects[i].pos = (Vector2){GetRandomValue(RA.x, (RA.x + RA.width) - OBJ_SIZE), GetRandomValue(RA.y, (RA.y + RA.height) - OBJ_SIZE)};}
	for(; i < 20; i++) {objects[i].type = PAPER; objects[i].pos = (Vector2){GetRandomValue(PA.x, (PA.x + PA.width) - OBJ_SIZE), GetRandomValue(PA.y, (PA.y + PA.height) - OBJ_SIZE)};}
	for(; i < 30; i++) {objects[i].type = SCISSORS; objects[i].pos = (Vector2){GetRandomValue(SA.x, (SA.x + SA.width) - OBJ_SIZE), GetRandomValue(SA.y, (SA.y + SA.height) - OBJ_SIZE)};}
}

// objects now move by randomly generating movement components
void moveObjects(Object objects[OBJ_COUNT], bool* reset, bool* game_end)
{
	for(int i = 0; i < OBJ_COUNT; i++)
	{
		objects[i].dx = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
		objects[i].dy = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
	}
	
	*reset = false;
	*game_end = false;
}

// returns the type of object that wins the entire game
enum Type checkWinner(Object objects[OBJ_COUNT])
{
	int rcnt = 0;
	int pcnt = 0;
	int scnt = 0;

	for(int i = 0; i < OBJ_COUNT; i++)
	{
		switch (objects[i].type)
		{
			case ROCK: rcnt++; if(rcnt == OBJ_COUNT) return ROCK; break;
			case PAPER: pcnt++; if(pcnt == OBJ_COUNT) return PAPER; break;
			case SCISSORS: scnt++; if(scnt == OBJ_COUNT) return SCISSORS; break;
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

// to customize bet and amount at stake
void placeBet(Player* player, char btnd[LIMIT], char cbd[LIMIT])
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

	// choose bet amount with slider
	if(!broke(player->balance)) GuiSliderBar((Rectangle){0, GA.height + 65, 100, 20}, "", "", &player->bet_amount, 0.10, player->balance);
}

// rewards or punishes player based on bet and bet amount
void payout(Player* player, char btnd[LIMIT], bool win, bool* game_end, char rd[LIMIT])
{
	strcpy(btnd, "RESET");
	if(win) {player->balance += (player->bet_amount * 2); strcpy(rd, WIN);}
	else {player->balance -= player->bet_amount; strcpy(rd, LOSE);} 

	*game_end = true;  
	player->bet = UNDF;
	player->bet_amount = (player->balance * 0.20f);
}

void init(Object objects[OBJ_COUNT], Player* player)
{
	SetTargetFPS(FPS);
	setObjects(objects);
	SetTraceLogLevel(LOG_ERROR);
	InitWindow(SCRA.width, SCRA.height, "Rock, Paper, Scissors Sim");

	player->bet = UNDF;
	player->balance = 100.00;
	player->bet_amount = 20.00;
}

int main()
{
	// flag if the game was reset
	bool reset = true;

	// flag if the game is over
    bool game_end = true;
    
	// button dialouge
	char btnd[LIMIT] = "START";

	// current bet dialouge
	char cbd[LIMIT];
	
	// result dialouge
	char rd[LIMIT];

    Player player;
    Object objects[OBJ_COUNT];
	Timer text_timer;

    init(objects, &player);

    while(!WindowShouldClose())
    {
		BeginDrawing();
			
			ClearBackground(BLACK);

			// drawing every object
			drawObjects(objects);
			
			if(!game_end)
			{
				// updates the position of all objects
				updateObjects(objects);
				
				// display the current bet the user has chosen
				DrawText(cbd, (SCRA.width / 2.0f) - (MeasureText(cbd, 30) / 2.0f), GA.height + 40, 30, RAYWHITE);
				
				// payout the player once the game ends and stop mov
				if(checkWinner(objects) != UNDF) payout(&player, btnd, (checkWinner(objects) == player.bet), &game_end, rd);
			}

			else
			{
				// able to place bet and set amount after resetting the board
				if(reset) placeBet(&player, btnd, cbd);

				// display wether player won or lost the bet
				else DrawText(rd, (SCRA.width / 2.0f) - (MeasureText(rd, 30) / 2.0), (SCRA.height / 2.0f), 30, RAYWHITE);

				// button to start and reset the game
				if(!broke(player.balance))
				{
					if(GuiButton((Rectangle){(SCRA.width / 2.0f) - 100, GA.height + ((SCRA.height - GA.height) / 2.0f) - 25, 200, 30}, btnd))
					{
						if(strcmp(btnd, "RESET") == 0) {setObjects(objects); reset = true;}

						if(strcmp(btnd, "START") == 0)
						{
							if(player.bet != UNDF) moveObjects(objects, &reset, &game_end);
							else StartTimer(&text_timer, TEXT_TIME);
						}
					}
				}
			}

			// no money to bet with dialouge
			if(broke(player.balance)) DrawText(NO_MONEY, (SCRA.width / 2.0f) - (MeasureText(NO_MONEY, 30) / 2.0f), (SCRA.height / 2.0f), 30, RAYWHITE);
			
			// reminds player to place a bet if they tried starting the game without placing one
			if(!TimerDone(text_timer)) DrawText(NO_BET, (SCRA.width / 2.0f) - (MeasureText(NO_BET, 30) / 2.0f), (GA.height / 2.0f), 30, RAYWHITE);
			
        EndDrawing();
    }

    CloseWindow();
    return 0;
}