#include "game.h"
#include "enemy.h"
#include "player.h"
#include "word.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

//--------------------------------------------------
// Game State & Timers
//--------------------------------------------------
float spawnTimer = 0.0f;
float spawnDelay = 2.5f;
bool gameOver = false;

int currentLevel = 1;
int wordsClearedThisLevel = 0;

//--------------------------------------------------
// Level Progression
//--------------------------------------------------
void CheckLevelProgression(void)
{
    // Progress level every 10 correctly typed words (cap at Level 10)
    if (wordsClearedThisLevel >= 10 && currentLevel < 10)
    {
        currentLevel++;
        wordsClearedThisLevel = 0;
        
        // Dynamically scale down the spawn arrival delay as WPM requirements rise
        int targetWPM = 20 + (currentLevel - 1) * 5;
        spawnDelay = 60.0f / targetWPM * 0.8f; 
    }
}

//--------------------------------------------------
// Initialize Game
//--------------------------------------------------
void InitGame(void)
{
    currentLevel = 1;
    wordsClearedThisLevel = 0;
    spawnDelay = 2.5f;
    gameOver = false;
    spawnTimer = 0.0f;
    
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
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
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
    ProcessTyping();

    // Collision check
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
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
    ClearBackground(BLACK);
    
    DrawPlayer();
    DrawEnemies();

    // UI & Text
    DrawText("TypingParo", 20, 20, 30, WHITE);

    char levelText[32];
    sprintf(levelText, "Level: %d / 10", currentLevel);
    DrawText(levelText, 20, 60, 20, RAYWHITE);
    
    char wpmText[32];
    sprintf(wpmText, "Target WPM: %d", 20 + (currentLevel - 1) * 5);
    DrawText(wpmText, 20, 85, 18, GREEN);

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
    UnloadPlayer(); 
}