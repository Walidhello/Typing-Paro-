#include "leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LEADERBOARD_FILE "assets/leaderboard.txt"

Leaderboard leaderboard;

static void PopulateDefaults(void)
{
    leaderboard.count = 1;

    strncpy(leaderboard.entries[0].name, "Walid", LEADERBOARD_NAME_LEN);
    leaderboard.entries[0].score = 4850;
    leaderboard.entries[0].level = 8;
    leaderboard.entries[0].accuracy = 98.4f;
    leaderboard.entries[0].wpm = 72;
}

void InitLeaderboard(void)
{
    LoadLeaderboard();
}

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

void SaveLeaderboard(void)
{
    FILE* fp = fopen(LEADERBOARD_FILE, "w");
    if (!fp) return;

    for (int i = 0; i < leaderboard.count; i++)\
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

int AddLeaderboardEntry(const char* name, int score, int level, float accuracy, int wpm)
{
    const char* cleanName = (name && strlen(name) > 0) ? name : "PILOT";

    // Keep leaderboard sorted descending by score
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
        return -1;
    }

    int newCount = (leaderboard.count < MAX_LEADERBOARD_ENTRIES) ? leaderboard.count + 1 : MAX_LEADERBOARD_ENTRIES;
    for (int i = newCount - 1; i > insertIdx; i--)
    {
        leaderboard.entries[i] = leaderboard.entries[i - 1];
    }

    strncpy(leaderboard.entries[insertIdx].name, cleanName, LEADERBOARD_NAME_LEN - 1);
    leaderboard.entries[insertIdx].name[LEADERBOARD_NAME_LEN - 1] = '\0';
    leaderboard.entries[insertIdx].score = score;
    leaderboard.entries[insertIdx].level = level;
    leaderboard.entries[insertIdx].accuracy = accuracy;
    leaderboard.entries[insertIdx].wpm = wpm;

    leaderboard.count = newCount;
    SaveLeaderboard();

    return insertIdx + 1;
}

void DrawLeaderboardScreen(int highlightRank)
{
    int screenW = 800;
    int screenH = 800;

    DrawRectangle(60, 50, screenW - 120, screenH - 100, (Color){ 12, 16, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ 60, 50, screenW - 120, screenH - 100 }, 2.0f, (Color){ 40, 160, 240, 220 });

    DrawRectangle(56, 46, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(screenW - 68, 46, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(56, screenH - 58, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(screenW - 68, screenH - 58, 12, 12, (Color){ 0, 240, 255, 255 });

    const char* title = "HALL OF FAME";
    int titleW = MeasureText(title, 34);
    DrawText(title, screenW / 2 - titleW / 2 + 2, 77, 34, (Color){ 0, 100, 180, 150 });
    DrawText(title, screenW / 2 - titleW / 2, 75, 34, (Color){ 0, 240, 255, 255 });

    const char* sub = "TOP INTERCEPTOR PILOTS // RECORDED AVERAGE WPM";
    int subW = MeasureText(sub, 15);
    DrawText(sub, screenW / 2 - subW / 2, 118, 15, (Color){ 160, 190, 220, 200 });

    DrawLine(90, 145, screenW - 90, 145, (Color){ 40, 100, 160, 180 });

    int colRank = 90;
    int colName = 170;
    int colScore = 370;
    int colLvl = 490;
    int colAcc = 570;
    int colWpm = 665;

    DrawText("RANK", colRank, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("PILOT", colName, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("SCORE", colScore, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("LEVEL", colLvl, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("ACCURACY", colAcc, 160, 16, (Color){ 120, 160, 200, 255 });
    DrawText("AVG WPM", colWpm, 160, 16, (Color){ 120, 160, 200, 255 });

    DrawLine(90, 185, screenW - 90, 185, (Color){ 30, 60, 100, 255 });

    int startY = 200;
    int rowHeight = 44;

    for (int i = 0; i < leaderboard.count; i++)
    {
        int rowY = startY + i * rowHeight;
        bool isCurrent = ((i + 1) == highlightRank);

        if (isCurrent)
        {
            DrawRectangle(80, rowY - 4, screenW - 160, rowHeight - 4, (Color){ 20, 70, 50, 200 });
            DrawRectangleLinesEx((Rectangle){ 80, rowY - 4, screenW - 160, rowHeight - 4 }, 1.5f, (Color){ 50, 255, 140, 255 });
        }
        else if (i % 2 == 1)
        {
            DrawRectangle(80, rowY - 4, screenW - 160, rowHeight - 4, (Color){ 16, 22, 38, 140 });
        }

        Color rankCol = (Color){ 180, 210, 240, 255 };
        char rankStr[8];
        snprintf(rankStr, sizeof(rankStr), "#%02d", i + 1);

        if (i == 0) rankCol = (Color){ 255, 215, 0, 255 };
        else if (i == 1) rankCol = (Color){ 210, 220, 230, 255 };
        else if (i == 2) rankCol = (Color){ 230, 140, 60, 255 };

        if (isCurrent) rankCol = (Color){ 50, 255, 140, 255 };

        DrawText(rankStr, colRank, rowY + 5, 18, rankCol);

        Color nameCol = isCurrent ? (Color){ 50, 255, 140, 255 } : WHITE;
        DrawText(leaderboard.entries[i].name, colName, rowY + 5, 18, nameCol);

        char scoreStr[16];
        snprintf(scoreStr, sizeof(scoreStr), "%d", leaderboard.entries[i].score);
        DrawText(scoreStr, colScore, rowY + 5, 18, (Color){ 0, 230, 255, 255 });

        char lvlStr[8];
        snprintf(lvlStr, sizeof(lvlStr), "LV %d", leaderboard.entries[i].level);
        DrawText(lvlStr, colLvl, rowY + 5, 18, (Color){ 200, 220, 240, 220 });

        char accStr[16];
        snprintf(accStr, sizeof(accStr), "%.1f%%", leaderboard.entries[i].accuracy);
        DrawText(accStr, colAcc, rowY + 5, 18, (Color){ 255, 220, 80, 255 });

        char wpmStr[16];
        snprintf(wpmStr, sizeof(wpmStr), "%d WPM", leaderboard.entries[i].wpm);
        DrawText(wpmStr, colWpm, rowY + 5, 18, (Color){ 80, 255, 150, 255 });
    }

    DrawLine(90, screenH - 120, screenW - 90, screenH - 120, (Color){ 40, 100, 160, 180 });

    Rectangle backBtn = { (float)(screenW / 2 - 130), (float)(screenH - 105), 260.0f, 42.0f };
    Vector2 mouse = GetMousePosition();
    bool backHover = CheckCollisionPointRec(mouse, backBtn);

    DrawRectangleRec(backBtn, backHover ? (Color){ 0, 190, 240, 255 } : (Color){ 20, 45, 80, 255 });
    DrawRectangleLinesEx(backBtn, 1.5f, backHover ? WHITE : (Color){ 0, 220, 255, 255 });

    const char* bText = "RETURN TO MENU [ESC]";
    int btW = MeasureText(bText, 15);
    DrawText(bText, screenW / 2 - btW / 2, screenH - 92, 15, backHover ? BLACK : WHITE);
}

void DrawLeaderboardMini(int startX, int startY, int width, int highlightRank)
{
    DrawRectangle(startX, startY, width, 180, (Color){ 14, 18, 30, 230 });
    DrawRectangleLinesEx((Rectangle){ (float)startX, (float)startY, (float)width, 180.0f }, 1.5f, (Color){ 40, 120, 200, 180 });

    DrawText("HALL OF FAME - TOP RECORDS", startX + 15, startY + 12, 16, (Color){ 0, 230, 255, 255 });
    DrawText("AVG WPM", startX + width - 210, startY + 14, 12, (Color){ 120, 160, 200, 220 });
    DrawText("SCORE", startX + width - 85, startY + 14, 12, (Color){ 120, 160, 200, 220 });

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

        char wpmMiniStr[16];
        snprintf(wpmMiniStr, sizeof(wpmMiniStr), "%d WPM", leaderboard.entries[i].wpm);
        DrawText(wpmMiniStr, startX + width - 215, rowY, 15, (Color){ 80, 255, 150, 255 });

        char scoreStr[16];
        snprintf(scoreStr, sizeof(scoreStr), "%d PTS", leaderboard.entries[i].score);
        int scoreW = MeasureText(scoreStr, 15);
        DrawText(scoreStr, startX + width - scoreW - 20, rowY, 15, (Color){ 0, 230, 255, 255 });
    }
}
