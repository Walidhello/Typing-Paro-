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
    {
        // Play Again logic
        if (IsKeyPressed(KEY_ENTER))
        {
            gameOver = false;
            spawnTimer = 0.0f;
            InitEnemies();
        }
        return;
    }

    spawnTimer += GetFrameTime();

    if(spawnTimer >= spawnDelay)
    {
        SpawnEnemy();
        spawnTimer = 0;
    }

    UpdateEnemies();
    
    // Process keyboard inputs for typing
    ProcessTyping();

    // Collision
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            // Adjust collision buffer to match the new image height if needed
            if(enemies[i].y >= player.position.y - (player.height / 2))
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
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 20, 40, RED);
        DrawText("Press ENTER to Play Again", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 30, 20, LIGHTGRAY);
    }
}

//--------------------------------------------------
// Unload
//--------------------------------------------------

void UnloadGame(void)
{
    UnloadPlayer(); // Free the player image texture from memory
}