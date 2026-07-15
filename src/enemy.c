#include "enemy.h"
#include "game.h"
#include "word.h"
#include "player.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int targetEnemy = -1; 
Enemy enemies[MAX_ENEMIES];

void InitEnemies(void)
{
    targetEnemy = -1; 
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
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
            
            int targetWPM = 20 + (currentLevel - 1) * 5;
            enemies[i].speed = 300.0f * GetFrameTime();
            
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

            int targetWPM = 20 + (currentLevel - 1) * 5;
            enemies[i].speed = 200.0f * GetFrameTime(); 

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

            strcpy(enemies[i].word, GetRandomWord());
            enemies[i].typedLetters = 0;
            break;
        }
    }
}

void UpdateEnemies(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            enemies[i].y += enemies[i].speed;

            if (enemies[i].type == E_MEDIUM_BOSS) {
                if (enemies[i].x < player.position.x) enemies[i].x += enemies[i].speed * 0.6f;
                else if (enemies[i].x > player.position.x) enemies[i].x -= enemies[i].speed * 0.6f;
            }

            if(enemies[i].y > SCREEN_HEIGHT + 50)
            {
                enemies[i].active = false;
                if(targetEnemy == i) targetEnemy = -1; 
            }
        }
    }
}

void ProcessTyping(void)
{
    // CHEAT LOGIC: Instantly skip level if the apostrophe key is pressed
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
        if (targetEnemy == -1) 
        {
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active && enemies[i].word[0] == key)
                {
                    targetEnemy = i;
                    enemies[i].typedLetters = 1;
                    
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