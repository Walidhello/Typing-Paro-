#include "game.h"
#include "enemy.h"
#include "player.h"
#include "word.h"
#include "gunship.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

//--------------------------------------------------
// Game State & Timers
//--------------------------------------------------
float spawnTimer = 0.0f;
float spawnDelay = 2.5f;
bool gameOver = false;
bool isPaused = false; // <-- NEW: Pause tracker

int currentLevel = 1;
int wordsClearedThisLevel = 0;

int totalChars = 0;
int correctChars = 0;
float typingTime = 0.0f;

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
    totalChars = 0;
    correctChars = 0;
    typingTime = 0.0f;
    gameOver = false;
    isPaused = false;
    spawnTimer = 0.0f;
    
    InitPlayer();
    InitGunship();
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

    // <-- NEW: Toggle Pause on ESCAPE
    if (IsKeyPressed(KEY_ESCAPE))
    {
        isPaused = !isPaused;
    }

    // <-- NEW: If game is paused, stop updating enemies and timers
    if (isPaused) return; 

    spawnTimer += GetFrameTime();

    if(spawnTimer >= spawnDelay)
    {
        SpawnEnemy();
        spawnTimer = 0;
    }

    UpdateEnemies();
    ProcessTyping();
    UpdateGunship();

    if (totalChars > 0)
    {
        typingTime += GetFrameTime();
    }

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
float GetAccuracy(void)
{
    if (totalChars == 0)
        return 100.0f;

    return ((float)correctChars / totalChars) * 100.0f;
}
float GetLiveWPM(void)
{
    if (typingTime <= 0.0f)
        return 0.0f;

    float minutes = typingTime / 60.0f;

    return ((float)correctChars / 5.0f) / minutes;
}
//--------------------------------------------------
// Draw Game
//--------------------------------------------------
void DrawGame(void)
{
    ClearBackground(BLACK);

    DrawRectangle(0,130,SCREEN_WIDTH,SCREEN_HEIGHT - 130,(Color){8, 8, 14, 255});

    DrawGunship();
    DrawEnemies();

    DrawRectangle(0, 0, SCREEN_WIDTH, 130, (Color){15, 15, 25, 255});
    DrawLine(0, 130, SCREEN_WIDTH, 130, DARKGRAY);

    // UI & Text
    DrawText("Typing Paro?", 20, 20, 30, WHITE);

    char levelText[32];
    sprintf(levelText, "Level: %02d / 10", currentLevel);
    int levelWidth = MeasureText(levelText, 22);
    DrawText(levelText, SCREEN_WIDTH - levelWidth - 25, 60, 20, RAYWHITE);
    
    // WPM card
    DrawRectangle(290, 20, 150, 80, (Color){25, 25, 40, 255});
    DrawRectangleLines(290, 20, 150, 80, DARKGRAY);

    DrawText("LIVE WPM", 305, 32, 16, LIGHTGRAY);

    char wpmText[32];
    sprintf(wpmText, "%.1f", GetLiveWPM());

    int wpmWidth = MeasureText(wpmText, 28);

    DrawText(wpmText,365 - wpmWidth / 2,58,28,GREEN);

    // Accuracy card
    DrawRectangle(460, 20, 150, 80, (Color){25, 25, 40, 255});
    DrawRectangleLines(460, 20, 150, 80, DARKGRAY);

    DrawText("ACCURACY", 475, 32, 16, LIGHTGRAY);

    char accuracyText[32];
    sprintf(accuracyText, "%.1f%%", GetAccuracy());

    int accuracyWidth = MeasureText(accuracyText, 28);

    DrawText(accuracyText,535 - accuracyWidth / 2,58,28,YELLOW);

    if(gameOver)
    {
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 128, SCREEN_HEIGHT/2 - 20, 40, RED);
        DrawText("Press ENTER to Play Again", SCREEN_WIDTH/2 - 145, SCREEN_HEIGHT/2 + 30, 20, LIGHTGRAY);
    }
    else if (isPaused) // <-- NEW: Pause Screen UI
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