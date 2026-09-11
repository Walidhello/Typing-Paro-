#include "enemy.h"
#include "game.h"
#include "word.h"
#include "player.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

int targetEnemy = -1; 
Enemy enemies[MAX_ENEMIES];

void InitEnemies(void)
{
    targetEnemy = -1; 
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
        enemies[i].vx = 0.0f;
        enemies[i].vy = 0.0f;
        enemies[i].headingToPlayer = false;
        enemies[i].pivotY = MEDIUM_ENEMY_PIVOT_Y;
    }
}

void SpawnMinions(float x, float y) 
{
    int spawned = 0;
    LoadWords("assets/words/easy.txt"); 
    
    for(int i = 0; i < MAX_ENEMIES && spawned < 3; i++) 
    {
        if(!enemies[i].active) 
        {
            enemies[i].active = true;
            enemies[i].type = E_NORMAL;
            enemies[i].x = x + (spawned - 1) * 70; 
            enemies[i].y = y;
            
            int targetWPM = 20;
            enemies[i].speed = (300.0f * targetWPM) / 3600.0f;
            enemies[i].vx = 0.0f;
            enemies[i].vy = enemies[i].speed;
            
            strcpy(enemies[i].word, GetRandomWord());
            enemies[i].typedLetters = 0;
            spawned++;
        }
    }
}


void SpawnEnemy(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active == false)
        {
            enemies[i].active = true;
            enemies[i].x = GetRandomValue(80, SCREEN_WIDTH - 80);
            enemies[i].y = -40;

            int targetWPM = 20;
            // + (currentLevel - 1) * 5;
            enemies[i].speed = (300.0f * targetWPM) / 3600.0f; 

            enemies[i].type = E_NORMAL;
            int pool = 1; 
            if (currentLevel >= 5) pool = 3;      
            else if (currentLevel >= 3) pool = 2; 

            int choice = GetRandomValue(1, pool);
            if (choice == 3) {
                enemies[i].type = E_HARD_BOSS;
                LoadWords("assets/words/hard.txt");
            } else if (choice == 2) {
                enemies[i].type = E_MEDIUM_BOSS;
                LoadWords("assets/words/medium.txt");
            } else {
                LoadWords("assets/words/easy.txt");
            }

            // All enemies start with a straight projection downward
            enemies[i].vx = 0.0f;
            enemies[i].vy = enemies[i].speed;
            enemies[i].headingToPlayer = false;
            enemies[i].pivotY = MEDIUM_ENEMY_PIVOT_Y;

            strcpy(enemies[i].word, GetRandomWord());
            enemies[i].typedLetters = 0;
            break;
        }
    }
}


void UpdateEnemies(void)
{
    // Normalize delta time to 60 FPS so your existing speed calculations remain unchanged
    float dt = GetFrameTime() * 60.0f; 

    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            if (enemies[i].type == E_MEDIUM_BOSS) 
            {
                // 1. Calculate direction to the player continuously
                float dx = player.position.x - enemies[i].x;
                float dy = player.position.y - enemies[i].y;
                float length = sqrtf(dx * dx + dy * dy);

                if (length > 0.0f) 
                {
                    // 2. Determine where the boss *wants* to go
                    float desired_vx = (dx / length) * enemies[i].speed;
                    float desired_vy = (dy / length) * enemies[i].speed;

                    // 3. Smoothly steer (Lerp) current velocity towards desired velocity
                    // Adjust 0.04f to change how wide or tight the fluid curve is
                    float smoothing = 0.04f;
                    enemies[i].vx += (desired_vx - enemies[i].vx) * smoothing * dt;
                    enemies[i].vy += (desired_vy - enemies[i].vy) * smoothing * dt;
                }
            }

            // Apply the velocity multiplied by delta time to eliminate micro-stutters
            enemies[i].x += enemies[i].vx * dt;
            enemies[i].y += enemies[i].vy * dt;

            // Despawn boundaries (expanded to accommodate curved trajectories)
            if(enemies[i].y > SCREEN_HEIGHT + 50 || 
               enemies[i].y < -150 || 
               enemies[i].x < -150 || 
               enemies[i].x > SCREEN_WIDTH + 150)
            {
                enemies[i].active = false;
                if(targetEnemy == i) targetEnemy = -1; 
            }
        }
    }
}


void ProcessTyping(void)
{
    // NEW CHEAT LOGIC: Instantly skip level if the apostrophe key is pressed
    if (IsKeyPressed(KEY_APOSTROPHE))
    {
        targetEnemy = -1; // Reset target
        SkipLevel();      // Progress level and clear enemies
        return;
    }
    // END CHEAT LOGIC

    int key = GetCharPressed();

    while (key > 0)
    {
        totalChars++;
        if (targetEnemy == -1) 
        {
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active && enemies[i].word[0] == key)
                {
                    targetEnemy = i;
                    enemies[i].typedLetters = 1;
                    correctChars++;
                    
                    if (enemies[i].typedLetters >= strlen(enemies[i].word))
                    {
                        if (enemies[i].type == E_HARD_BOSS) {
                            SpawnMinions(enemies[i].x, enemies[i].y);
                        }
                        enemies[i].active = false;
                        targetEnemy = -1;
                        wordsClearedThisLevel++;
                        CheckLevelProgression();
                    }
                    break; 
                }
            }
        }
        else 
        {
            Enemy* e = &enemies[targetEnemy];
            if (e->word[e->typedLetters] == key)
            {
                e->typedLetters++;
                correctChars++;
                
                if (e->typedLetters >= strlen(e->word))
                {
                    if (e->type == E_HARD_BOSS) {
                        SpawnMinions(e->x, e->y);
                    }
                    e->active = false;
                    targetEnemy = -1;
                    wordsClearedThisLevel++;
                    CheckLevelProgression();
                }
            }
        }
        key = GetCharPressed(); 
    }
}

void DrawEnemies(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            Color boxColor = RED;
            if (enemies[i].type == E_MEDIUM_BOSS) boxColor = ORANGE;
            if (enemies[i].type == E_HARD_BOSS) boxColor = PURPLE;

            DrawRectangle(enemies[i].x - 20, enemies[i].y - 20, 40, 40, boxColor);
            DrawRectangle(enemies[i].x - 35, enemies[i].y - 45, 70, 20, DARKGRAY);

            const char* remainingText = &enemies[i].word[enemies[i].typedLetters];
            Color textColor = (i == targetEnemy) ? GREEN : WHITE; 

            DrawText(
                remainingText,
                enemies[i].x - MeasureText(remainingText, 20) / 2,
                enemies[i].y - 43,
                20,
                textColor
            );
        }
    }
}