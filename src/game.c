#include "game.h"
#include "enemy.h"
#include "player.h"
#include "word.h"
#include "raylib.h"
#include <stdbool.h>

//--------------------------------------------------
// Player Position
//--------------------------------------------------



//--------------------------------------------------
// Spawn Timer
//--------------------------------------------------

float spawnTimer = 0.0f;
float spawnDelay = 2.0f;

//--------------------------------------------------
// Game Over
//--------------------------------------------------

bool gameOver = false;

//--------------------------------------------------
// Initialize Game
//--------------------------------------------------

void InitGame(void)
{
    InitPlayer();
    LoadWords("assets/words/easy.txt");
    InitEnemies();

    SpawnEnemy();
}

//--------------------------------------------------
// Update Game
//--------------------------------------------------

void UpdateGame(void)
{
    if(gameOver)
        return;

    //--------------------------------
    // Spawn Enemies
    //--------------------------------

    spawnTimer += GetFrameTime();

    if(spawnTimer >= spawnDelay)
    {
        SpawnEnemy();
        spawnTimer = 0;
    }

    //--------------------------------
    // Move Enemies
    //--------------------------------

    UpdateEnemies();

    //--------------------------------
    // Collision
    //--------------------------------

    for(int i=0;i<MAX_ENEMIES;i++)
    {
        if(enemies[i].active)
        {
            if(enemies[i].y >= player.position.y - 20)
            {
                gameOver = true;
            }
        }
    }
}

//--------------------------------------------------
// Draw Game
//--------------------------------------------------

void DrawGame(void)
{
    //--------------------------------
    // Background
    //--------------------------------

    ClearBackground(BLACK);

    //--------------------------------
    // Player
    //--------------------------------

    DrawPlayer();

    //--------------------------------
    // Draw Enemies
    //--------------------------------

    DrawEnemies();

    //--------------------------------
    // Title
    //--------------------------------

    DrawText(
        "TypingParo",
        20,
        20,
        30,
        WHITE
    );

    //--------------------------------
    // Game Over
    //--------------------------------

    if(gameOver)
    {
        DrawText(
            "GAME OVER",
            SCREEN_WIDTH/2-100,
            SCREEN_HEIGHT/2,
            40,
            RED
        );
    }
}

//--------------------------------------------------
// Unload
//--------------------------------------------------

void UnloadGame(void)
{

}