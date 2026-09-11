#include "game.h"
#include "enemy.h"
#include "player.h"
#include "word.h"
#include "gunship.h"
#include "shooting.h"
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

int totalChars = 0;
int correctChars = 0;

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
        
        InitEnemies();
        InitShooting();
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
    gameOver = false;
    isPaused = false;
    spawnTimer = 0.0f;
    
    InitPlayer();
    InitGunship();
    InitShooting();
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
    UpdateGunship();
    UpdateShooting();

    // Collision check
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active && !enemies[i].isDying)
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

//--------------------------------------------------
// Draw Game
//--------------------------------------------------
void DrawGame(void)
{
    ClearBackground(BLACK);

    // Apply Screen Shake Camera to battlefield
    Vector2 shake = GetScreenShakeOffset();
    Camera2D camera = { 0 };
    camera.offset = shake;
    camera.zoom = 1.0f;

    BeginMode2D(camera);

    // Deep space battlefield background
    DrawRectangle(0, 130, SCREEN_WIDTH, SCREEN_HEIGHT - 130, (Color){8, 8, 14, 255});

    // Space grid lines
    for (int y = 180; y < SCREEN_HEIGHT; y += 80)
    {
        DrawLine(0, y, SCREEN_WIDTH, y, (Color){18, 20, 30, 255});
    }
    for (int x = 100; x < SCREEN_WIDTH; x += 100)
    {
        DrawLine(x, 130, x, SCREEN_HEIGHT, (Color){18, 20, 30, 255});
    }

    // Draw active projectiles (under enemies)
    DrawShootingProjectiles();

    // Draw player's gunship
    DrawGunship();

    // Draw enemies
    DrawEnemies();

    // Draw shooting visual effects (muzzle flashes, hit sparks, explosions, shockwaves, text)
    DrawShootingEffects();

    EndMode2D();

    // =====================================================
    // UI HUD HEADER (Stable, unaffected by camera shake)
    // =====================================================
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

    DrawText("TARGET WPM", 305, 32, 16, LIGHTGRAY);

    char wpmText[32];
    sprintf(wpmText, "%d", 20 + (currentLevel - 1) * 5);
    int wpmWidth = MeasureText(wpmText, 28);
    DrawText(wpmText, 365 - wpmWidth / 2, 58, 28, GREEN);

    // Accuracy card
    DrawRectangle(460, 20, 150, 80, (Color){25, 25, 40, 255});
    DrawRectangleLines(460, 20, 150, 80, DARKGRAY);

    DrawText("ACCURACY", 475, 32, 16, LIGHTGRAY);

    char accuracyText[32];
    sprintf(accuracyText, "%.1f%%", GetAccuracy());
    int accuracyWidth = MeasureText(accuracyText, 28);
    DrawText(accuracyText, 535 - accuracyWidth / 2, 58, 28, YELLOW);

    if(gameOver)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 128, SCREEN_HEIGHT/2 - 20, 40, RED);
        DrawText("Press ENTER to Play Again", SCREEN_WIDTH/2 - 145, SCREEN_HEIGHT/2 + 30, 20, LIGHTGRAY);
    }
    else if (isPaused)
    {
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
