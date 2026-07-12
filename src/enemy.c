#include "enemy.h"
#include "game.h"
#include "word.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int targetEnemy = -1; // Tracks which enemy the player is currently typing
//--------------------------------------------------
// Enemy Array
//--------------------------------------------------
Enemy enemies[MAX_ENEMIES];

//--------------------------------------------------
// Temporary Words
// (Later these will come from easy.txt)
//--------------------------------------------------


//--------------------------------------------------
// Initialize Enemies
//--------------------------------------------------
void InitEnemies(void)
{
    targetEnemy = -1; // Reset target on init
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
    }
}


//--------------------------------------------------
// Spawn One Enemy
//--------------------------------------------------
void SpawnEnemy(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active == false)
        {
            enemies[i].active = true;

            enemies[i].x = GetRandomValue(80, SCREEN_WIDTH - 80);

            enemies[i].y = -40;

            enemies[i].speed = GetRandomValue(2,4);

            strcpy(enemies[i].word,
       GetRandomWord());

            enemies[i].typedLetters = 0;

            break;
        }
    }
}

//--------------------------------------------------
// Update Enemies
//--------------------------------------------------
void UpdateEnemies(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            enemies[i].y += enemies[i].speed;

            if(enemies[i].y > SCREEN_HEIGHT + 50)
            {
                enemies[i].active = false;
                if(targetEnemy == i) targetEnemy = -1; // Reset target if it falls off screen
            }
        }
    }
}

void ProcessTyping(void)
{
    int key = GetCharPressed();

    while (key > 0)
    {
        if (targetEnemy == -1) // Look for a new enemy to target
        {
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active && enemies[i].word[0] == key)
                {
                    targetEnemy = i;
                    enemies[i].typedLetters = 1;
                    
                    if (enemies[i].typedLetters >= strlen(enemies[i].word))
                    {
                        enemies[i].active = false;
                        targetEnemy = -1;
                    }
                    break; 
                }
            }
        }
        else // Continue typing the targeted enemy
        {
            Enemy* e = &enemies[targetEnemy];
            if (e->word[e->typedLetters] == key)
            {
                e->typedLetters++;
                
                if (e->typedLetters >= strlen(e->word))
                {
                    e->active = false;
                    targetEnemy = -1;
                }
            }
        }
        key = GetCharPressed(); // Fetch next key in queue if typing fast
    }
}

//--------------------------------------------------
// Draw Enemies
//--------------------------------------------------
void DrawEnemies(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            DrawRectangle(enemies[i].x - 20, enemies[i].y - 20, 40, 40, RED);
            DrawRectangle(enemies[i].x - 35, enemies[i].y - 45, 70, 20, DARKGRAY);

            // Only draw the untyped portion of the word
            const char* remainingText = &enemies[i].word[enemies[i].typedLetters];
            Color textColor = (i == targetEnemy) ? GREEN : WHITE; // Highlight targeted word

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