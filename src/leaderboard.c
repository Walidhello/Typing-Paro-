#include "leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LEADERBOARD_FILE "assets/leaderboard.txt"

Leaderboard leaderboard;

//--------------------------------------------------
// Pre-populate Default High Scores
//--------------------------------------------------
static void PopulateDefaults(void)
{
    leaderboard.count = 5;

    strncpy(leaderboard.entries[0].name, "CYBER_ACE", LEADERBOARD_NAME_LEN);
    leaderboard.entries[0].score = 4850;
    leaderboard.entries[0].level = 8;
    leaderboard.entries[0].accuracy = 98.4f;
    leaderboard.entries[0].wpm = 72;

    strncpy(leaderboard.entries[1].name, "VALKYRIE", LEADERBOARD_NAME_LEN);
    leaderboard.entries[1].score = 3920;
    leaderboard.entries[1].level = 7;
    leaderboard.entries[1].accuracy = 96.1f;
    leaderboard.entries[1].wpm = 65;

    strncpy(leaderboard.entries[2].name, "NEO_PILOT", LEADERBOARD_NAME_LEN);
    leaderboard.entries[2].score = 3100;
    leaderboard.entries[2].level = 6;
    leaderboard.entries[2].accuracy = 94.8f;
    leaderboard.entries[2].wpm = 58;

    strncpy(leaderboard.entries[3].name, "NOVA_GUNNER", LEADERBOARD_NAME_LEN);
    leaderboard.entries[3].score = 2450;
    leaderboard.entries[3].level = 5;
    leaderboard.entries[3].accuracy = 93.2f;
    leaderboard.entries[3].wpm = 52;

    strncpy(leaderboard.entries[4].name, "ROOKIE_ONE", LEADERBOARD_NAME_LEN);
    leaderboard.entries[4].score = 1500;
    leaderboard.entries[4].level = 3;
    leaderboard.entries[4].accuracy = 91.0f;
    leaderboard.entries[4].wpm = 40;
}

//--------------------------------------------------
// Initialize Leaderboard
//--------------------------------------------------
void InitLeaderboard(void)
{
    LoadLeaderboard();
}

//--------------------------------------------------
// Load Leaderboard from Disk
//--------------------------------------------------
void LoadLeaderboard(void)
{
    leaderboard.count = 0;

    FILE* fp = fopen(LEADERBOARD_FILE, "r");
    if (!fp)
    {
        PopulateDefaults();
        SaveLeaderboard();
        return;
    }

    char line[128];
    while (fgets(line, sizeof(line), fp) && leaderboard.count < MAX_LEADERBOARD_ENTRIES)
    {
        char name[LEADERBOARD_NAME_LEN] = "";
        int score = 0;
        int level = 1;
        float accuracy = 0.0f;
        int wpm = 0;

        if (sscanf(line, "%23[^,],%d,%d,%f,%d", name, &score, &level, &accuracy, &wpm) == 5)
        {
            strncpy(leaderboard.entries[leaderboard.count].name, name, LEADERBOARD_NAME_LEN - 1);
            leaderboard.entries[leaderboard.count].name[LEADERBOARD_NAME_LEN - 1] = '\0';
            leaderboard.entries[leaderboard.count].score = score;
            leaderboard.entries[leaderboard.count].level = level;
            leaderboard.entries[leaderboard.count].accuracy = accuracy;
            leaderboard.entries[leaderboard.count].wpm = wpm;
            leaderboard.count++;
        }
    }

    fclose(fp);

    if (leaderboard.count == 0)
    {
        PopulateDefaults();
        SaveLeaderboard();
    }
}

//--------------------------------------------------
// Save Leaderboard to Disk
//--------------------------------------------------
void SaveLeaderboard(void)
{
    FILE* fp = fopen(LEADERBOARD_FILE, "w");
    if (!fp) return;

    for (int i = 0; i < leaderboard.count; i++)
    {
        fprintf(fp, "%s,%d,%d,%.1f,%d\n",
                leaderboard.entries[i].name,
                leaderboard.entries[i].score,
                leaderboard.entries[i].level,
                leaderboard.entries[i].accuracy,
                leaderboard.entries[i].wpm);
    }

    fclose(fp);
}

//--------------------------------------------------
// Add New Entry to Leaderboard (Sorted descending)
//--------------------------------------------------
int AddLeaderboardEntry(const char* name, int score, int level, float accuracy, int wpm)
{
    const char* cleanName = (name && strlen(name) > 0) ? name : "PILOT";

    // Find insertion index
    int insertIdx = -1;
    for (int i = 0; i < leaderboard.count; i++)
    {
        if (score > leaderboard.entries[i].score)
        {
            insertIdx = i;
            break;
        }
    }

    if (insertIdx == -1 && leaderboard.count < MAX_LEADERBOARD_ENTRIES)
    {
        insertIdx = leaderboard.count;
    }

    if (insertIdx == -1)
    {
        return -1; // Not in top 10
    }

    // Shift entries down
    int newCount = (leaderboard.count < MAX_LEADERBOARD_ENTRIES) ? leaderboard.count + 1 : MAX_LEADERBOARD_ENTRIES;
    for (int i = newCount - 1; i > insertIdx; i--)
    {
        leaderboard.entries[i] = leaderboard.entries[i - 1];
    }

    // Insert new entry
    strncpy(leaderboard.entries[insertIdx].name, cleanName, LEADERBOARD_NAME_LEN - 1);
    leaderboard.entries[insertIdx].name[LEADERBOARD_NAME_LEN - 1] = '\0';
    leaderboard.entries[insertIdx].score = score;
    leaderboard.entries[insertIdx].level = level;
    leaderboard.entries[insertIdx].accuracy = accuracy;
    leaderboard.entries[insertIdx].wpm = wpm;

    leaderboard.count = newCount;
    SaveLeaderboard();

    return insertIdx + 1; // 1-indexed rank
}

//--------------------------------------------------
// Draw Fullscreen Leaderboard
//--------------------------------------------------
void DrawLeaderboardScreen(int highlightRank)
{
    int screenW = 800;
    int screenH = 800;

    // Glowing cyberpunk background panel
    DrawRectangle(60, 50, screenW - 120, screenH - 100, (Color){ 12, 16, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ 60, 50, screenW - 120, screenH - 100 }, 2.0f, (Color){ 40, 160, 240, 220 });

    // Decorative corner notches
    DrawRectangle(56, 46, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(screenW - 68, 46, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(56, screenH - 58, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(screenW - 68, screenH - 58, 12, 12, (Color){ 0, 240, 255, 255 });

    // Header
    const char* title = "HALL OF FAME";
    int titleW = MeasureText(title, 34);
    DrawText(title, screenW / 2 - titleW / 2 + 2, 77, 34, (Color){ 0, 100, 180, 150 });
    DrawText(title, screenW / 2 - titleW / 2, 75, 34, (Color){ 0, 240, 255, 255 });

    const char* sub = "TOP INTERCEPTOR PILOTS";
    int subW = MeasureText(sub, 16);
    DrawText(sub, screenW / 2 - subW / 2, 118, 16, (Color){ 160, 190, 220, 200 });

    DrawLine(90, 145, screenW - 90, 145, (Color){ 40, 100, 160, 180 });

    // Column Headers
    int colRank = 90;
    int colName = 170;
    int colScore = 380;
    int colLvl = 510;
    int colAcc = 590;
    int colWpm = 680;

    DrawText("RANK", colRank, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("PILOT", colName, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("SCORE", colScore, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("LEVEL", colLvl, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("ACCURACY", colAcc, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("WPM", colWpm, 160, 16, (Color){ 120, 160, 200, 255 });

    DrawLine(90, 185, screenW - 90, 185, (Color){ 30, 60, 100, 255 });

    // Render entries
    int startY = 200;
    int rowHeight = 44;

    for (int i = 0; i < leaderboard.count; i++)
    {
        int rowY = startY + i * rowHeight;
        bool isCurrent = ((i + 1) == highlightRank);

        // Row background
        if (isCurrent)
        {
            DrawRectangle(80, rowY - 4, screenW - 160, rowHeight - 4, (Color){ 20, 70, 50, 200 });
            DrawRectangleLinesEx((Rectangle){ 80, rowY - 4, screenW - 160, rowHeight - 4 }, 1.5f, (Color){ 50, 255, 140, 255 });
        }
        else if (i % 2 == 1)
        {
            DrawRectangle(80, rowY - 4, screenW - 160, rowHeight - 4, (Color){ 16, 22, 38, 140 });
        }

        // Rank styling
        Color rankCol = (Color){ 180, 210, 240, 255 };
        char rankStr[8];
        snprintf(rankStr, sizeof(rankStr), "#%02d", i + 1);

        if (i == 0) rankCol = (Color){ 255, 215, 0, 255 };      // Gold
        else if (i == 1) rankCol = (Color){ 210, 220, 230, 255 }; // Silver
        else if (i == 2) rankCol = (Color){ 230, 140, 60, 255 };  // Bronze

        if (isCurrent) rankCol = (Color){ 50, 255, 140, 255 };

        DrawText(rankStr, colRank, rowY + 5, 18, rankCol);

        // Name
        Color nameCol = isCurrent ? (Color){ 50, 255, 140, 255 } : WHITE;
        DrawText(leaderboard.entries[i].name, colName, rowY + 5, 18, nameCol);

        // Score
        char scoreStr[16];
        snprintf(scoreStr, sizeof(scoreStr), "%d", leaderboard.entries[i].score);
        DrawText(scoreStr, colScore, rowY + 5, 18, (Color){ 0, 230, 255, 255 });

        // Level
        char lvlStr[8];
        snprintf(lvlStr, sizeof(lvlStr), "LV %d", leaderboard.entries[i].level);
        DrawText(lvlStr, colLvl, rowY + 5, 18, (Color){ 200, 220, 240, 220 });

        // Accuracy
        char accStr[16];
        snprintf(accStr, sizeof(accStr), "%.1f%%", leaderboard.entries[i].accuracy);
        DrawText(accStr, colAcc, rowY + 5, 18, (Color){ 255, 220, 80, 255 });

        // WPM
        char wpmStr[8];
        snprintf(wpmStr, sizeof(wpmStr), "%d", leaderboard.entries[i].wpm);
        DrawText(wpmStr, colWpm, rowY + 5, 18, (Color){ 80, 255, 150, 255 });
    }

    // Return button / footer
    DrawLine(90, screenH - 120, screenW - 90, screenH - 120, (Color){ 40, 100, 160, 180 });

    const char* hint = "PRESS [ESC] OR [ENTER] TO RETURN TO MENU";
    int hintW = MeasureText(hint, 16);
    DrawText(hint, screenW / 2 - hintW / 2, screenH - 95, 16, (Color){ 140, 180, 220, 220 });
}

//--------------------------------------------------
// Draw Compact Mini-Leaderboard for Game Over Screen
//--------------------------------------------------
void DrawLeaderboardMini(int startX, int startY, int width, int highlightRank)
{
    DrawRectangle(startX, startY, width, 180, (Color){ 14, 18, 30, 230 });
    DrawRectangleLinesEx((Rectangle){ startX, startY, width, 180 }, 1.5f, (Color){ 40, 120, 200, 180 });

    DrawText("HALL OF FAME - TOP RECORDS", startX + 15, startY + 12, 16, (Color){ 0, 230, 255, 255 });
    DrawLine(startX + 15, startY + 34, startX + width - 15, startY + 34, (Color){ 30, 60, 100, 200 });

    int maxShow = (leaderboard.count < 5) ? leaderboard.count : 5;
    for (int i = 0; i < maxShow; i++)
    {
        int rowY = startY + 44 + i * 26;
        bool isCurrent = ((i + 1) == highlightRank);

        Color rowCol = isCurrent ? (Color){ 50, 255, 140, 255 } : WHITE;
        Color rankCol = (i == 0) ? (Color){ 255, 215, 0, 255 } : (isCurrent ? (Color){ 50, 255, 140, 255 } : LIGHTGRAY);

        char rankStr[8];
        snprintf(rankStr, sizeof(rankStr), "#%d", i + 1);
        DrawText(rankStr, startX + 20, rowY, 15, rankCol);

        DrawText(leaderboard.entries[i].name, startX + 65, rowY, 15, rowCol);

        char scoreStr[16];
        snprintf(scoreStr, sizeof(scoreStr), "%d PTS", leaderboard.entries[i].score);
        int scoreW = MeasureText(scoreStr, 15);
        DrawText(scoreStr, startX + width - scoreW - 20, rowY, 15, (Color){ 0, 230, 255, 255 });
    }
}
