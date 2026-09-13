#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_LEADERBOARD_ENTRIES 10
#define LEADERBOARD_NAME_LEN 24

typedef struct {
    char name[LEADERBOARD_NAME_LEN];
    int score;
    int level;
    float accuracy;
    int wpm;
} LeaderboardEntry;

typedef struct {
    LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
    int count;
} Leaderboard;

extern Leaderboard leaderboard;

void InitLeaderboard(void);
void LoadLeaderboard(void);
void SaveLeaderboard(void);
int AddLeaderboardEntry(const char* name, int score, int level, float accuracy, int wpm);

void DrawLeaderboardScreen(int highlightRank);
void DrawLeaderboardMini(int startX, int startY, int width, int highlightRank);

#endif
