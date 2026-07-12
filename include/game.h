#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "enemy.h"

//------------------------------------
// Screen Size
//------------------------------------
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 700

//------------------------------------
// Maximum number of enemies
//------------------------------------


//------------------------------------
// Game Functions
//------------------------------------
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UnloadGame(void);

#endif