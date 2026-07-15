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
bool isPaused = false; 

int currentLevel = 1;
int wordsClearedThisLevel = 0;

//--------------------------------------------------
// Level Progression & Cheats
//--------------------------------------------------
void CheckLevelProgression(void)
{
    if (wordsClearedThisLevel >= 10 && currentLevel < 10)
    {
        currentLevel++;
        wordsClearedThisLevel = 0;
        
        int targetWPM = 20 + (currentLevel - 1) * 5;
        spawnDelay = 60.0f / targetWPM * 0.8f; 
    }
}

void SkipLevel(void)
{
    if (currentLevel < 10)
    {
        currentLevel++;
        wordsClearedThisLevel = 0;
        
        int targetWPM = 20 + (currentLevel - 1) * 5;
        spawnDelay = 60.0f / targetWPM * 0.8f; 
        
        // Clear the screen and spawn a fresh enemy for the new level
        InitEnemies();
        spawnTimer = 0.0f;
        SpawnEnemy();
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
    isPaused = false;
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

    // Toggle Pause on ESCAPE
    if (IsKeyPressed(KEY_ESCAPE))
    {
        isPaused = !isPaused;
    }

    
    if (isPaused) return; 

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
    
    //for future use////////////////
    char wpmText[32];
    //<--want to check the wpm for future version
    

    if(gameOver)
    {
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 20, 40, RED);
        DrawText("Press ENTER to Play Again", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 30, 20, LIGHTGRAY);
    }
    else if (isPaused) // Pause Screen UI
    {
        // Draws a semi-transparent black box over the whole screen
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
        DrawText("PAUSED", SCREEN_WIDTH/2 - 70, SCREEN_HEIGHT/2 - 20, 40, YELLOW);
        DrawText("Press ESC to Resume", SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 30, 20, LIGHTGRAY);
    }
}

//--------------------------------------------------
// Unload
//--------------------------------------------------
void UnloadGame(void)
{
    UnloadPlayer(); 
}