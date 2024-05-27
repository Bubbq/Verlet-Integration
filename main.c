#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

const int FPS = 60;
const int SCREEN_SIZE = 600;
const Rectangle SCRA = {0, 0, SCREEN_SIZE, SCREEN_SIZE}; // screen area

const int OBJ_SIZE = 16;
const int OBJ_COUNT = 30;
const float OBJ_SPEED = 1.00;

// rock, paper, and scissor spawn areas
const Rectangle RA = {0, 0, (SCREEN_SIZE / 2.0f), (SCREEN_SIZE / 2.0f)};
const Rectangle PA = {(SCREEN_SIZE  / 2.0f), 0, (SCREEN_SIZE / 2.0f), (SCREEN_SIZE / 2.0f)};
const Rectangle SA = {(SCREEN_SIZE  / 4.0f), (SCREEN_SIZE / 2.0f), (SCREEN_SIZE / 2.0f), (SCREEN_SIZE / 2.0f)};

enum type
{
    ROCK = 0,
    PAPER = 1,
    SCISSORS = 2,
};

// represents one module in the game
typedef struct
{
    // current position
    Vector2 pos;
    // movement components
    float dx, dy;
    // objects type (rock, paper, or scissors) 
    enum type type;
    // png image to b displayed
    Texture2D texture;
} Object;

// randomly generates the positions of objects  
void generatePositions(Object objects[OBJ_COUNT])
{
    // based on the type, randomize obj position by that types area
    for(int i = 0; i < OBJ_COUNT; i++)
    {
        Vector2 pos;
        switch (objects[i].type)
        {
            case ROCK: pos = (Vector2){GetRandomValue(RA.x, (RA.x + RA.width) - OBJ_SIZE), GetRandomValue(RA.y, (RA.y + RA.height) - OBJ_SIZE)}; break;
            case PAPER: pos = (Vector2){GetRandomValue(PA.x, (PA.x + PA.width) - OBJ_SIZE), GetRandomValue(PA.y, (PA.y + PA.height) - OBJ_SIZE)}; break;
            case SCISSORS: pos = (Vector2){GetRandomValue(SA.x, (SA.x + SA.width) - OBJ_SIZE), GetRandomValue(SA.y, (SA.y + SA.height) - OBJ_SIZE)}; break;
            default:
                break;
        }

        objects[i].pos = pos;

        // randomly generate movement compoenents
        objects[i].dx = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
        objects[i].dy = (GetRandomValue(0, 1) == 1) ? OBJ_SPEED : -OBJ_SPEED;
    }
}

// draws all objects on the screen
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

// collision with screen borders 
void screenCollision(Object* obj)
{
    if((obj->pos.y < 0) || ((obj->pos.y + OBJ_SIZE) > SCREEN_SIZE)) obj->dy *= -1; // top/bottom of screen
    if((obj->pos.x < 0) || ((obj->pos.x + OBJ_SIZE) > SCREEN_SIZE)) obj->dx *= -1; // left/right side
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
            if(j->type == ROCK) i->type = ROCK;// loss
            else if(j->type == PAPER) j->type = SCISSORS; // win
            break;

        default:
            break;
    }
}

// collision from obj i with obj j
void objectCollision(Object* i, Object* j)
{
    // Velocities before collision
    Vector2 vi = {i->dx, i->dy};
    Vector2 vj = {j->dx, j->dy};

    // finds the position of obj i w.r.t obj j, then, scales this resulting vector with magnitude 1 (x^2 + y^2)^1/2 = 1
    Vector2 normal = Vector2Normalize(Vector2Subtract(i->pos, j->pos));

    // tangential vector, role is to b perpendicular to the normal
    Vector2 tangent = (Vector2){-normal.y, normal.x};

    // scale these vectors by the magnitude of the velocity vectors of obj i and j before their collision
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

void updateObjects(Object objects[OBJ_COUNT])
{
    for(int i = 0; i < OBJ_COUNT; i++)
    {
        // Move by component magnitude
        objects[i].pos = Vector2Add(objects[i].pos, (Vector2){objects[i].dx, objects[i].dy});

        // Screen collisions
        screenCollision(&objects[i]);

        // Area of ith object
        Rectangle obj_area = {objects[i].pos.x, objects[i].pos.y, OBJ_SIZE, OBJ_SIZE};

        // Collisions with other objects
        for(int j = 0; j < OBJ_COUNT; j++)
        {
            // Don't check collision with itself or objects of same type
            if(objects[i].type == objects[j].type) continue;

            Rectangle potential_col = {objects[j].pos.x, objects[j].pos.y, OBJ_SIZE, OBJ_SIZE};
            
            if(CheckCollisionRecs(obj_area, potential_col)) objectCollision(&objects[i], &objects[j]);
        }
    }
}

void init(Object objects[OBJ_COUNT])
{
    SetTargetFPS(FPS);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "RPS-Game");

    // start with 10 of each type
    int i = 0;
    for(; i < 10; i++) objects[i].type = ROCK;
    for(; i < 20; i++) objects[i].type = PAPER;
    for(; i < 30; i++) objects[i].type = SCISSORS;

    // generate positions for these objects
    generatePositions(objects);
}

void deinit()
{
    CloseWindow();
}

int main()
{
    Object objects[OBJ_COUNT];
    init(objects);

    while(!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BLACK);
            updateObjects(objects);
            drawObjects(objects);
        EndDrawing();
    }

    deinit();
    return 0;
}