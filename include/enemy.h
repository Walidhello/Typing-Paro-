#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include <stdbool.h>

//------------------------------------
// Maximum word length
//------------------------------------
#define WORD_LENGTH 32
#define MAX_ENEMIES 20
//------------------------------------
// Enemy Structure
//------------------------------------
typedef struct
{
    float x;
    float y;

    float speed;

    char word[WORD_LENGTH];

    int typedLetters;

    bool active;

} Enemy;
extern Enemy enemies[MAX_ENEMIES];

//------------------------------------
// Enemy Functions
//------------------------------------
void InitEnemies(void);

void SpawnEnemy(void);

void UpdateEnemies(void);

void DrawEnemies(void);

void ProcessTyping(void);

#endif