#include "enemy.h"
#include "game.h"
#include "word.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
            }
        }
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
            // Enemy body
            DrawRectangle(
                enemies[i].x - 20,
                enemies[i].y - 20,
                40,
                40,
                RED
            );

            // Word Background
            DrawRectangle(
                enemies[i].x - 35,
                enemies[i].y - 45,
                70,
                20,
                DARKGRAY
            );

            // Word
            DrawText(
                enemies[i].word,
                enemies[i].x - MeasureText(enemies[i].word,20)/2,
                enemies[i].y - 43,
                20,
                WHITE
            );
        }
    }
}