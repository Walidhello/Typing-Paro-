#include "leaderboard.h"
#include <stdio.h>
#include <string.h>

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
        LeaderboardEntry* e = &leaderboard.entries[leaderboard.count];
        if (sscanf(line, "%23[^,],%d,%d,%f,%d", e->name, &e->score, &e->level, &e->accuracy, &e->wpm) == 5)
        {
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

    for (int i = 0; i < leaderboard.count; i++)
    {
        LeaderboardEntry* e = &leaderboard.entries[i];
        fprintf(fp, "%s,%d,%d,%.1f,%d\n", e->name, e->score, e->level, e->accuracy, e->wpm);
    }
    fclose(fp);
}

int AddLeaderboardEntry(const char* name, int score, int level, float accuracy, int wpm)
{
    const char* cleanName = (name && name[0]) ? name : "PILOT";

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
        insertIdx = leaderboard.count;

    if (insertIdx == -1) return -1;

    int newCount = (leaderboard.count < MAX_LEADERBOARD_ENTRIES) ? leaderboard.count + 1 : MAX_LEADERBOARD_ENTRIES;
    for (int i = newCount - 1; i > insertIdx; i--)
        leaderboard.entries[i] = leaderboard.entries[i - 1];

    LeaderboardEntry* e = &leaderboard.entries[insertIdx];
    strncpy(e->name, cleanName, LEADERBOARD_NAME_LEN - 1);
    e->name[LEADERBOARD_NAME_LEN - 1] = '\0';
    e->score = score;
    e->level = level;
    e->accuracy = accuracy;
    e->wpm = wpm;

    leaderboard.count = newCount;
    SaveLeaderboard();
    return insertIdx + 1;
}

void DrawLeaderboardScreen(int highlightRank)
{
    int screenW = 800, screenH = 800;
    DrawRectangle(60, 50, screenW - 120, screenH - 100, (Color){ 12, 16, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ 60, 50, screenW - 120, screenH - 100 }, 2.0f, (Color){ 40, 160, 240, 220 });

    int corners[4][2] = {{ 56, 46 }, { screenW - 68, 46 }, { 56, screenH - 58 }, { screenW - 68, screenH - 58 }};
    for (int i = 0; i < 4; i++)
        DrawRectangle(corners[i][0], corners[i][1], 12, 12, (Color){ 0, 240, 255, 255 });

    const char* title = "HALL OF FAME";
    int titleW = MeasureText(title, 34);
    DrawText(title, screenW / 2 - titleW / 2 + 2, 77, 34, (Color){ 0, 100, 180, 150 });
    DrawText(title, screenW / 2 - titleW / 2, 75, 34, (Color){ 0, 240, 255, 255 });

    const char* sub = "TOP INTERCEPTOR PILOTS // RECORDED AVERAGE WPM";
    DrawText(sub, screenW / 2 - MeasureText(sub, 15) / 2, 118, 15, (Color){ 160, 190, 220, 200 });
    DrawLine(90, 145, screenW - 90, 145, (Color){ 40, 100, 160, 180 });

    const char* headers[] = { "RANK", "PILOT", "SCORE", "LEVEL", "ACCURACY", "AVG WPM" };
    int cols[] = { 90, 170, 370, 490, 570, 665 };
    for (int i = 0; i < 6; i++)
        DrawText(headers[i], cols[i], 160, 16, (Color){ 120, 160, 200, 255 });

    DrawLine(90, 185, screenW - 90, 185, (Color){ 30, 60, 100, 255 });

    int startY = 200, rowHeight = 44;
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

        Color rankCol = (i == 0) ? (Color){ 255, 215, 0, 255 } :
                        (i == 1) ? (Color){ 210, 220, 230, 255 } :
                        (i == 2) ? (Color){ 230, 140, 60, 255 } : (Color){ 180, 210, 240, 255 };
        if (isCurrent) rankCol = (Color){ 50, 255, 140, 255 };

        char buf[32];
        snprintf(buf, sizeof(buf), "#%02d", i + 1);
        DrawText(buf, cols[0], rowY + 5, 18, rankCol);
        DrawText(leaderboard.entries[i].name, cols[1], rowY + 5, 18, isCurrent ? (Color){ 50, 255, 140, 255 } : WHITE);
        snprintf(buf, sizeof(buf), "%d", leaderboard.entries[i].score);
        DrawText(buf, cols[2], rowY + 5, 18, (Color){ 0, 230, 255, 255 });
        snprintf(buf, sizeof(buf), "LV %d", leaderboard.entries[i].level);
        DrawText(buf, cols[3], rowY + 5, 18, (Color){ 200, 220, 240, 220 });
        snprintf(buf, sizeof(buf), "%.1f%%", leaderboard.entries[i].accuracy);
        DrawText(buf, cols[4], rowY + 5, 18, (Color){ 255, 220, 80, 255 });
        snprintf(buf, sizeof(buf), "%d WPM", leaderboard.entries[i].wpm);
        DrawText(buf, cols[5], rowY + 5, 18, (Color){ 80, 255, 150, 255 });
    }

    DrawLine(90, screenH - 120, screenW - 90, screenH - 120, (Color){ 40, 100, 160, 180 });

    Rectangle backBtn = { (float)(screenW / 2 - 130), (float)(screenH - 105), 260.0f, 42.0f };
    bool backHover = CheckCollisionPointRec(GetMousePosition(), backBtn);
    DrawRectangleRec(backBtn, backHover ? (Color){ 0, 190, 240, 255 } : (Color){ 20, 45, 80, 255 });
    DrawRectangleLinesEx(backBtn, 1.5f, backHover ? WHITE : (Color){ 0, 220, 255, 255 });
    const char* bText = "RETURN TO MENU [ESC]";
    DrawText(bText, screenW / 2 - MeasureText(bText, 15) / 2, screenH - 92, 15, backHover ? BLACK : WHITE);
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

        char buf[24];
        snprintf(buf, sizeof(buf), "#%d", i + 1);
        DrawText(buf, startX + 20, rowY, 15, rankCol);
        DrawText(leaderboard.entries[i].name, startX + 65, rowY, 15, rowCol);

        snprintf(buf, sizeof(buf), "%d WPM", leaderboard.entries[i].wpm);
        DrawText(buf, startX + width - 215, rowY, 15, (Color){ 80, 255, 150, 255 });

        snprintf(buf, sizeof(buf), "%d PTS", leaderboard.entries[i].score);
        DrawText(buf, startX + width - MeasureText(buf, 15) - 20, rowY, 15, (Color){ 0, 230, 255, 255 });
    }
}
