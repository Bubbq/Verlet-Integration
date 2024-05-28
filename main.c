#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

const int FPS = 60;
const int GAME_AREA = 500;

// screen & game area
const Rectangle SCRA = {0, 0, 500, 600};
const Rectangle GA = {0,0, GAME_AREA, GAME_AREA};

const int OBJ_SIZE = 16;
const int OBJ_COUNT = 30;
const float OBJ_SPEED = 2.00;

// rock, paper, and scissor spawn areas
const Rectangle RA = {0, 0, (GAME_AREA / 2.0f), (GAME_AREA / 2.0f)};
const Rectangle PA = {(GAME_AREA  / 2.0f), 0, (GAME_AREA / 2.0f), (GAME_AREA / 2.0f)};
const Rectangle SA = {(GAME_AREA  / 4.0f), (GAME_AREA / 2.0f), (GAME_AREA / 2.0f), (GAME_AREA / 2.0f)};

enum type
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
	// current position
	Vector2 pos;
	// movement components
	float dx, dy;
	// objects type (rock, paper, or scissors)
	enum type type;
	// png image to b displayed
	Texture2D texture;
} Object;

typedef struct
{
	enum type bet;
	float balance;
	float bet_amount;
} Player;

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
void findWinner(Object* i, Object* j)
{
	switch (i->type)
	{
		case ROCK:
			if(j->type == PAPER) i->type = PAPER; // loss
			else if(j->type == SCISSORS) j->type = ROCK; // win
			break;
			
		case PAPER:
			if(j->type == SCISSORS) i->type = SCISSORS; // win
			else if(j->type == ROCK) j->type = PAPER; // loss
			break;

		case SCISSORS:
			if(j->type == ROCK) i->type = ROCK; // loss
			else if(j->type == PAPER) j->type = SCISSORS; // win
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
	findWinner(i, j);
}

// to move each object within the screen
void updateObjects(Object objects[OBJ_COUNT])
{
	for(int i = 0; i < OBJ_COUNT; i++)
	{
		if(!objects[i].move) continue;

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
	for(; i < 10; i++) {objects[i].type = ROCK; objects[i].pos = (Vector2){GetRandomValue(RA.x, (RA.x + RA.width) - OBJ_SIZE), GetRandomValue(RA.y, (RA.y + RA.height) - OBJ_SIZE)}; objects[i].dx = objects[i].dy = 0;}
	for(; i < 20; i++) {objects[i].type = PAPER; objects[i].pos = (Vector2){GetRandomValue(PA.x, (PA.x + PA.width) - OBJ_SIZE), GetRandomValue(PA.y, (PA.y + PA.height) - OBJ_SIZE)}; objects[i].dx = objects[i].dy = 0;}
	for(; i < 30; i++) {objects[i].type = SCISSORS; objects[i].pos = (Vector2){GetRandomValue(SA.x, (SA.x + SA.width) - OBJ_SIZE), GetRandomValue(SA.y, (SA.y + SA.height) - OBJ_SIZE)}; objects[i].dx = objects[i].dy = 0;}
}

// objects now move by randomly generating movement components
void startGame(Object objects[OBJ_COUNT])
{
	for(int i = 0; i < OBJ_COUNT; i++)
	{
		objects[i].move = true;
		objects[i].dx = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
		objects[i].dy = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
	}
}

// returns the type of object that wins the entire game
enum type checkWinner(Object objects[OBJ_COUNT])
{
	int rcnt = 0; int pcnt = 0; int scnt = 0;

	for(int i = 0; i < OBJ_COUNT; i++)
	{
		switch (objects[i].type)
		{
			case ROCK: rcnt++; break;
			case PAPER: pcnt++; break;
			case SCISSORS: scnt++; break;
			default:   
				break;
		}
	}

	// printf("%d %d %d \n", rcnt, pcnt, scnt);
	if(rcnt == OBJ_COUNT) return ROCK;
	else if(pcnt == OBJ_COUNT) return PAPER;
	else if(scnt == OBJ_COUNT) return SCISSORS;
	else return UNDF;
}

void init(Object objects[OBJ_COUNT], Player* player)
{
	SetTargetFPS(FPS);
	SetTraceLogLevel(LOG_ERROR);
	InitWindow(SCRA.width, SCRA.height, "Rock, Paper, Scissors Sim");
	setObjects(objects);


	player->bet = UNDF;
	player->balance = 100.00;
	player->bet_amount = 10.00;
}

int main()
{
   Player player;
   char* btn = "";
   enum type wobjt = UNDF;
   Object objects[OBJ_COUNT];
   void (*btn_exe)(Object[OBJ_COUNT]) = startGame;
  
   init(objects, &player);

   while(!WindowShouldClose())
   {
		if((wobjt = checkWinner(objects)) != UNDF)
		{
			btn = "RESET";
			for(int i = 0; i < OBJ_COUNT; i++) objects[i].move = false;
			btn_exe = setObjects;
		}

		else {btn = "START"; btn_exe = startGame;}

		// choosing bet
		if(GuiButton((Rectangle){0, GA.height + 15, 20, 20}, "R")) player.bet = ROCK;
		if(GuiButton((Rectangle){0, GA.height + 45, 20, 20}, "P")) player.bet = PAPER;
		if(GuiButton((Rectangle){0, GA.height + 75, 20, 20}, "S")) player.bet = SCISSORS;

		BeginDrawing();
			updateObjects(objects);
			drawObjects(objects);
			DrawFPS(0, 0);

			if((GuiButton((Rectangle){(SCRA.width / 2.0f) - 25, GA.height + ((SCRA.height - GA.height) / 2.0f) - 25, 50, 50}, btn))) btn_exe(objects);
			
			ClearBackground(BLACK);

		EndDrawing();
   }

   CloseWindow();
   return 0;
}