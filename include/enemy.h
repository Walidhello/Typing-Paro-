#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include <stdbool.h>

#define WORD_LENGTH 32
#define MAX_ENEMIES 20
#define MEDIUM_ENEMY_PIVOT_Y 250.0f

typedef enum {
    E_NORMAL,
    E_MEDIUM_BOSS,
    E_HARD_BOSS
} EnemyType;

typedef struct {
    bool active;
    EnemyType type;
    float x;
    float y;
    float speed;
    float vx; 
    float vy; 
    char word[WORD_LENGTH];
    int typedLetters;
    float pivotY;
    bool headingToPlayer;
    float hitFlashTimer;
    float minionSpawnTimer;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];
extern int targetEnemy;

void InitEnemies(void);
void SpawnEnemy(void);
void SpawnMinions(float x, float y);
void UpdateEnemies(void);
void DrawEnemies(void);
void ProcessTyping(void);

#endif
