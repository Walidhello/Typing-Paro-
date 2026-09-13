#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define MAX_SONIC_WAVES 3

typedef enum {
    STATE_MENU,
    STATE_LAUNCH_ANIMATION,
    STATE_PLAYING,
    STATE_LEVEL_CLEAR,
    STATE_GAMEOVER,
    STATE_LEADERBOARD
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
extern bool isPaused;
extern int sonicWavesRemaining;
extern float spawnTimer;

void InitGame(void);
void StartNewGame(void);
void UpdateGame(void);
void DrawGame(void);
void UnloadGame(void);

void CheckLevelProgression(void);
void SkipLevel(void);
float GetAccuracy(void);
bool TriggerSonicWave(void);

int GetLiveWPM(void);
int GetAverageWPM(void);
void RecordCorrectChar(void);

#endif
