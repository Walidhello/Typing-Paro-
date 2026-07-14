#ifndef GAME_H
#define GAME_H

#include "raylib.h"

//------------------------------------
// Screen Size
//------------------------------------
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 700

//------------------------------------
// Global Level Variables
//------------------------------------
extern int currentLevel;
extern int wordsClearedThisLevel;

//------------------------------------
// Game Functions
//------------------------------------
void CheckLevelProgression(void);
void SkipLevel(void); //
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UnloadGame(void);

#endif