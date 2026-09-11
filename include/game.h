#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

typedef enum {
    STATE_MENU,
    STATE_LEADERBOARD,
    STATE_PLAYING,
    STATE_GAMEOVER
} GameState;

extern GameState gameState;
extern char pilotName[24];
extern int currentScore;
extern int currentCombo;
extern int maxCombo;

extern int currentLevel;
extern int wordsClearedThisLevel;
extern int totalChars;
extern int correctChars;

void InitGame(void);
void StartNewGame(void);
void UpdateGame(void);
void DrawGame(void);
void UnloadGame(void);

void CheckLevelProgression(void);
void SkipLevel(void);
float GetAccuracy(void);

#endif
