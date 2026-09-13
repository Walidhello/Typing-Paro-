#include "enemy.h"
#include "game.h"
#include "word.h"
#include "gunship.h"
#include "shooting.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

int targetEnemy = -1;
Enemy enemies[MAX_ENEMIES];
static int comboStreak = 0;

void InitEnemies(void)
{
    targetEnemy = -1;
    comboStreak = 0;
    memset(enemies, 0, sizeof(enemies));
}

void SpawnMinions(float x, float y)
{
    int activeCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active) activeCount++;
    if (activeCount >= 10) return;

    LoadWords("assets/words/easy.txt");

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].active = true;
            enemies[i].type = E_NORMAL;

            static int bayToggle = 0;
            float bayX = (bayToggle == 0) ? (x - 42.0f) : (x + 42.0f);
            float driftVx = (bayToggle == 0) ? -0.45f : 0.45f;
            bayToggle = 1 - bayToggle;

            enemies[i].x = bayX;
            enemies[i].y = y;

            int targetWPM = 20 + (currentLevel - 1) * 3;
            enemies[i].speed = (300.0f * (float)targetWPM) / 3600.0f;
            enemies[i].vx = driftVx;
            enemies[i].vy = enemies[i].speed;

            strncpy(enemies[i].word, GetRandomWord(), WORD_LENGTH - 1);
            enemies[i].word[WORD_LENGTH - 1] = '\0';
            enemies[i].typedLetters = 0;
            enemies[i].hitFlashTimer = 0.0f;
            enemies[i].minionSpawnTimer = 0.0f;
            enemies[i].errorTimer = 0.0f;

            CreateHitSparks((Vector2){ bayX, y }, (Color){ 0, 240, 255, 255 }, 8);
            break;
        }
    }
}

void SpawnEnemy(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].active = true;
            enemies[i].x = (float)GetRandomValue(90, SCREEN_WIDTH - 90);
            enemies[i].y = -40.0f;

            int targetWPM = 20 + (currentLevel - 1) * 5;
            enemies[i].speed = (300.0f * (float)targetWPM) / 3600.0f;

            enemies[i].type = E_NORMAL;
            int pool = (currentLevel >= 5) ? 3 : ((currentLevel >= 3) ? 2 : 1);
            int choice = GetRandomValue(1, pool);

            if (choice == 3)
            {
                enemies[i].type = E_HARD_BOSS;
                LoadWords("assets/words/hard.txt");
                enemies[i].speed *= 0.75f;
            }
            else if (choice == 2)
            {
                enemies[i].type = E_MEDIUM_BOSS;
                LoadWords("assets/words/medium.txt");
            }
            else
            {
                LoadWords("assets/words/easy.txt");
            }

            enemies[i].vx = 0.0f;
            enemies[i].vy = enemies[i].speed;
            enemies[i].hitFlashTimer = 0.0f;
            enemies[i].minionSpawnTimer = 0.0f;
            enemies[i].errorTimer = 0.0f;

            strncpy(enemies[i].word, GetRandomWord(), WORD_LENGTH - 1);
            enemies[i].word[WORD_LENGTH - 1] = '\0';
            enemies[i].typedLetters = 0;
            break;
        }
    }
}

void UpdateEnemies(void)
{
    float dt = GetFrameTime();
    float simDt = dt * 60.0f;

    if (targetEnemy >= 0 && (targetEnemy >= MAX_ENEMIES || !enemies[targetEnemy].active))
        targetEnemy = -1;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            enemies[i].hitFlashTimer = fmaxf(0.0f, enemies[i].hitFlashTimer - dt);
            enemies[i].errorTimer = fmaxf(0.0f, enemies[i].errorTimer - dt);

            if (enemies[i].type == E_HARD_BOSS)
            {
                enemies[i].minionSpawnTimer += dt;
                if (enemies[i].minionSpawnTimer >= 2.0f)
                {
                    enemies[i].minionSpawnTimer = 0.0f;
                    SpawnMinions(enemies[i].x, enemies[i].y + 15.0f);
                }
            }

            enemies[i].x += enemies[i].vx * simDt;
            enemies[i].y += enemies[i].vy * simDt;

            if (enemies[i].y > SCREEN_HEIGHT + 60 || enemies[i].y < -150 ||
                enemies[i].x < -150 || enemies[i].x > SCREEN_WIDTH + 150)
            {
                enemies[i].active = false;
                if (targetEnemy == i) targetEnemy = -1;
            }
        }
    }
}

void ProcessTyping(void)
{
    if (IsKeyPressed(KEY_APOSTROPHE))
    {
        targetEnemy = -1;
        SkipLevel();
        return;
    }

    if (IsKeyPressed(KEY_SPACE))
        TriggerSonicWave();

    if (targetEnemy >= 0 && (targetEnemy >= MAX_ENEMIES || !enemies[targetEnemy].active))
        targetEnemy = -1;

    int key = GetCharPressed();
    bool anyKeyProcessed = false;
    bool anyKeyHit = false;

    while (key > 0)
    {
        if (key == 32 || key == ' ')
        {
            TriggerSonicWave();
            key = GetCharPressed();
            continue;
        }

        anyKeyProcessed = true;
        totalChars++;
        int inputChar = tolower(key);

        if (targetEnemy == -1)
        {
            int bestCandidate = -1;
            float closestDistSq = 999999999.0f;
            float px = gunship.position.x, py = gunship.position.y;

            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active && tolower((unsigned char)enemies[i].word[0]) == inputChar)
                {
                    float dx = enemies[i].x - px, dy = enemies[i].y - py;
                    float distSq = dx * dx + dy * dy;
                    if (distSq < closestDistSq)
                    {
                        closestDistSq = distSq;
                        bestCandidate = i;
                    }
                }
            }
            if (bestCandidate != -1) targetEnemy = bestCandidate;
        }

        if (targetEnemy != -1)
        {
            Enemy* e = &enemies[targetEnemy];
            if (e->active && tolower((unsigned char)e->word[e->typedLetters]) == inputChar)
            {
                e->typedLetters++;
                RecordCorrectChar();
                anyKeyHit = true;

                comboStreak++;
                currentCombo = 1 + (comboStreak / 8);
                if (currentCombo > 5) currentCombo = 5;
                if (currentCombo > maxCombo) maxCombo = currentCombo;
                currentScore += 15 * currentLevel * currentCombo;

                e->hitFlashTimer = 0.12f;

                bool isDestroyed = (e->typedLetters >= (int)strlen(e->word));
                if (isDestroyed)
                {
                    currentScore += 120 * currentLevel * currentCombo;
                    FireLaserAtPosition((Vector2){ e->x, e->y }, e->type, e->word, true);
                    e->active = false;
                    targetEnemy = -1;
                    wordsClearedThisLevel++;
                    CheckLevelProgression();
                }
                else
                {
                    FireLaserAtPosition((Vector2){ e->x, e->y }, e->type, e->word, false);
                }
            }
        }
        key = GetCharPressed();
    }

    if (anyKeyProcessed && !anyKeyHit)
    {
        comboStreak = 0;
        currentCombo = 1;
        TriggerMisfire();

        int errCandidate = (targetEnemy >= 0 && targetEnemy < MAX_ENEMIES && enemies[targetEnemy].active) ? targetEnemy : -1;
        if (errCandidate == -1)
        {
            float closestDistSq = 999999999.0f;
            float px = gunship.position.x, py = gunship.position.y;
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active)
                {
                    float dx = enemies[i].x - px, dy = enemies[i].y - py;
                    float distSq = dx * dx + dy * dy;
                    if (distSq < closestDistSq)
                    {
                        closestDistSq = distSq;
                        errCandidate = i;
                    }
                }
            }
        }

        if (errCandidate != -1)
        {
            enemies[errCandidate].errorTimer = 0.35f;
            AddFloatText((Vector2){ enemies[errCandidate].x, enemies[errCandidate].y - 48.0f }, "MISS", (Color){ 255, 60, 60, 255 }, 16);
        }
    }
}

static void DrawNormalEnemyShip(float x, float y, bool isHit)
{
    if (isHit)
    {
        DrawTriangle((Vector2){x, y + 20}, (Vector2){x - 14, y - 14}, (Vector2){x + 14, y - 14}, WHITE);
        DrawTriangle((Vector2){x - 5, y - 8}, (Vector2){x - 22, y + 8}, (Vector2){x - 14, y - 14}, WHITE);
        DrawTriangle((Vector2){x + 5, y - 8}, (Vector2){x + 14, y - 14}, (Vector2){x + 22, y + 8}, WHITE);
        DrawCircleLines((int)x, (int)y, 22.0f, SKYBLUE);
        return;
    }

    float flicker = sinf(GetTime() * 40.0f + x) * 3.0f;
    DrawTriangle((Vector2){x - 5, y - 16}, (Vector2){x + 5, y - 16}, (Vector2){x, y - 24 - flicker}, (Color){255, 100, 20, 255});
    DrawTriangle((Vector2){x - 3, y - 16}, (Vector2){x + 3, y - 16}, (Vector2){x, y - 20 - flicker * 0.7f}, (Color){255, 220, 50, 255});
    DrawRectangle((int)x - 6, (int)y - 17, 12, 3, (Color){45, 50, 60, 255});

    DrawTriangle((Vector2){x - 4, y - 6}, (Vector2){x - 22, y + 10}, (Vector2){x - 12, y - 14}, (Color){130, 22, 35, 255});
    DrawLine((int)(x - 4), (int)(y - 6), (int)(x - 22), (int)(y + 10), (Color){255, 50, 70, 255});

    DrawTriangle((Vector2){x + 4, y - 6}, (Vector2){x + 12, y - 14}, (Vector2){x + 22, y + 10}, (Color){130, 22, 35, 255});
    DrawLine((int)(x + 4), (int)(y - 6), (int)(x + 22), (int)(y + 10), (Color){255, 50, 70, 255});

    DrawLine((int)(x - 20), (int)(y + 4), (int)(x - 20), (int)(y + 14), (Color){200, 210, 225, 255});
    DrawLine((int)(x + 20), (int)(y + 4), (int)(x + 20), (int)(y + 14), (Color){200, 210, 225, 255});

    DrawTriangle((Vector2){x, y + 20}, (Vector2){x - 13, y - 14}, (Vector2){x + 13, y - 14}, (Color){38, 22, 28, 255});
    DrawTriangle((Vector2){x, y + 16}, (Vector2){x - 9, y - 10}, (Vector2){x + 9, y - 10}, (Color){185, 30, 45, 255});
    DrawLine((int)x, (int)(y - 12), (int)x, (int)(y + 14), (Color){255, 120, 140, 255});

    DrawTriangle((Vector2){x, y + 6}, (Vector2){x - 4, y - 3}, (Vector2){x + 4, y - 3}, (Color){255, 215, 0, 255});
    DrawCircle((int)x, (int)y, 2.0f, WHITE);
}

static void DrawMediumBossShip(float x, float y, bool isHit)
{
    if (isHit)
    {
        DrawTriangle((Vector2){x, y + 28}, (Vector2){x - 26, y - 22}, (Vector2){x + 26, y - 22}, WHITE);
        DrawTriangle((Vector2){x - 12, y - 6}, (Vector2){x - 38, y + 10}, (Vector2){x - 24, y - 24}, WHITE);
        DrawTriangle((Vector2){x + 12, y - 6}, (Vector2){x + 24, y - 24}, (Vector2){x + 38, y + 10}, WHITE);
        DrawCircleLines((int)x, (int)y, 38.0f, (Color){255, 220, 100, 255});
        return;
    }

    float flicker = sinf(GetTime() * 45.0f + x) * 4.0f;
    DrawTriangle((Vector2){x - 24, y - 24}, (Vector2){x - 14, y - 24}, (Vector2){x - 19, y - 35 - flicker}, (Color){255, 120, 20, 255});
    DrawTriangle((Vector2){x - 22, y - 24}, (Vector2){x - 16, y - 24}, (Vector2){x - 19, y - 30 - flicker * 0.7f}, (Color){255, 220, 60, 255});
    DrawRectangle((int)x - 25, (int)y - 25, 12, 4, (Color){50, 55, 65, 255});

    DrawTriangle((Vector2){x + 14, y - 24}, (Vector2){x + 24, y - 24}, (Vector2){x + 19, y - 35 - flicker}, (Color){255, 120, 20, 255});
    DrawTriangle((Vector2){x + 16, y - 24}, (Vector2){x + 22, y - 24}, (Vector2){x + 19, y - 30 - flicker * 0.7f}, (Color){255, 220, 60, 255});
    DrawRectangle((int)x + 13, (int)y - 25, 12, 4, (Color){50, 55, 65, 255});

    DrawTriangle((Vector2){x - 10, y - 6}, (Vector2){x - 38, y + 12}, (Vector2){x - 24, y - 22}, (Color){150, 85, 15, 255});
    DrawTriangle((Vector2){x - 14, y - 4}, (Vector2){x - 34, y + 8}, (Vector2){x - 22, y - 16}, (Color){215, 130, 25, 255});
    DrawLineEx((Vector2){x - 10, y - 6}, (Vector2){x - 38, y + 12}, 2.0f, (Color){255, 190, 50, 255});

    DrawTriangle((Vector2){x + 10, y - 6}, (Vector2){x + 24, y - 22}, (Vector2){x + 38, y + 12}, (Color){150, 85, 15, 255});
    DrawTriangle((Vector2){x + 14, y - 4}, (Vector2){x + 22, y - 16}, (Vector2){x + 34, y + 8}, (Color){215, 130, 25, 255});
    DrawLineEx((Vector2){x + 10, y - 6}, (Vector2){x + 38, y + 12}, 2.0f, (Color){255, 190, 50, 255});

    DrawRectangle((int)x - 32, (int)y + 2, 6, 16, (Color){40, 45, 55, 255});
    DrawLine((int)(x - 29), (int)(y + 18), (int)(x - 29), (int)(y + 24), (Color){255, 200, 70, 255});
    DrawRectangle((int)x + 26, (int)y + 2, 6, 16, (Color){40, 45, 55, 255});
    DrawLine((int)(x + 29), (int)(y + 18), (int)(x + 29), (int)(y + 24), (Color){255, 200, 70, 255});

    DrawTriangle((Vector2){x, y + 28}, (Vector2){x - 26, y - 20}, (Vector2){x + 26, y - 20}, (Color){32, 35, 45, 255});
    DrawTriangle((Vector2){x, y + 22}, (Vector2){x - 18, y - 14}, (Vector2){x + 18, y - 14}, (Color){195, 115, 20, 255});

    DrawRectangle((int)x - 10, (int)y + 4, 20, 7, (Color){20, 25, 35, 255});
    DrawRectangle((int)x - 8, (int)y + 5, 16, 5, (Color){255, 220, 60, 255});

    DrawCircle((int)x, (int)y - 6, 7.0f, (Color){40, 50, 70, 255});
    DrawCircle((int)x, (int)y - 6, 4.5f, (Color){255, 160, 20, 255});
    DrawCircle((int)x, (int)y - 6, 2.0f, WHITE);
    DrawCircle((int)x - 37, (int)y + 11, 2.5f, (Color){255, 210, 50, 255});
    DrawCircle((int)x + 37, (int)y + 11, 2.5f, (Color){255, 210, 50, 255});
}

static void DrawHardBossStation(float x, float y, bool isHit)
{
    if (isHit)
    {
        DrawPoly((Vector2){x, y}, 8, 30.0f, 0.0f, WHITE);
        DrawRectangle((int)x - 52, (int)y - 12, 104, 24, WHITE);
        DrawRectangle((int)x - 12, (int)y - 38, 24, 76, WHITE);
        DrawCircleLines((int)x, (int)y, 54.0f, (Color){255, 80, 240, 255});
        return;
    }

    float time = (float)GetTime();

    for (int side = -1; side <= 1; side += 2)
    {
        int sx = (side == -1) ? ((int)x - 44) : ((int)x + 18);
        DrawRectangle(sx, (int)y - 38, 26, 12, (Color){18, 45, 85, 255});
        DrawRectangleLines(sx, (int)y - 38, 26, 12, (Color){0, 180, 255, 200});
        DrawLine(sx + 13, (int)y - 38, sx + 13, (int)y - 26, (Color){0, 220, 255, 180});
        DrawLineEx((Vector2){x + side * 20, y - 18}, (Vector2){sx + 13, y - 26}, 2.5f, (Color){60, 65, 85, 255});
    }

    DrawRectangle((int)x - 48, (int)y - 8, 96, 16, (Color){38, 32, 50, 255});
    DrawRectangle((int)x - 8, (int)y - 34, 16, 68, (Color){38, 32, 50, 255});
    DrawLineEx((Vector2){x - 46, y}, (Vector2){x + 46, y}, 1.5f, (Color){0, 230, 255, 200});
    DrawLineEx((Vector2){x, y - 32}, (Vector2){x, y + 32}, 1.5f, (Color){0, 230, 255, 200});

    for (int side = -1; side <= 1; side += 2)
    {
        int podX = (side == -1) ? ((int)x - 52) : ((int)x + 34);
        DrawRectangle(podX, (int)y - 16, 18, 32, (Color){26, 20, 36, 255});
        DrawRectangleLinesEx((Rectangle){(float)podX, y - 16, 18, 32}, 1.5f, (Color){150, 45, 195, 255});
        DrawRectangle(podX + 2, (int)y + 9, 14, 4, (Color){0, 240, 255, 255});
        DrawLine(podX + 4, (int)y - 12, podX + 4, (int)y + 7, (Color){255, 215, 0, 220});
        DrawLine(podX + 14, (int)y - 12, podX + 14, (int)y + 7, (Color){255, 215, 0, 220});

        DrawLineEx((Vector2){x + side * 18, y + 26}, (Vector2){x + side * 18, y + 42}, 2.0f, (Color){180, 190, 215, 255});
        DrawCircle((int)(x + side * 18), (int)y + 42, 2.0f, RED);
    }

    DrawPoly((Vector2){x, y}, 8, 25.0f, 22.5f, (Color){30, 24, 42, 255});
    DrawPoly((Vector2){x, y}, 8, 20.0f, 22.5f, (Color){90, 32, 115, 255});

    float pulse = 0.85f + 0.15f * sinf(time * 6.0f);
    DrawCircle((int)x, (int)y, 11.0f * pulse, (Color){190, 35, 230, 255});
    DrawCircle((int)x, (int)y, 7.0f * pulse, (Color){255, 80, 240, 255});
    DrawCircle((int)x, (int)y, 3.5f, WHITE);

    bool blink = ((int)(time * 3.5f) % 2 == 0);
    Color blinkCol = blink ? (Color){0, 240, 255, 255} : (Color){0, 70, 100, 255};
    DrawCircle((int)x - 52, (int)y, 3.0f, blinkCol);
    DrawCircle((int)x + 52, (int)y, 3.0f, blinkCol);
}

void DrawEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            float drawX = enemies[i].x;
            float drawY = enemies[i].y;
            bool isHit = (enemies[i].hitFlashTimer > 0.0f);

            float badgeY = drawY - 38.0f;
            float pad = 26.0f;
            float bLen = 8.0f;

            if (enemies[i].type == E_HARD_BOSS)
            {
                DrawHardBossStation(drawX, drawY, isHit);
                badgeY = drawY - 56.0f;
                pad = 56.0f;
                bLen = 14.0f;
            }
            else if (enemies[i].type == E_MEDIUM_BOSS)
            {
                DrawMediumBossShip(drawX, drawY, isHit);
                badgeY = drawY - 48.0f;
                pad = 42.0f;
                bLen = 11.0f;
            }
            else
            {
                DrawNormalEnemyShip(drawX, drawY, isHit);
            }

            float shakeX = (enemies[i].errorTimer > 0.0f) ?
                (sinf(enemies[i].errorTimer * 55.0f) * 7.0f * (enemies[i].errorTimer / 0.35f)) : 0.0f;
            float badgeDrawX = drawX + shakeX;

            if (i == targetEnemy)
            {
                Color reticleColor = (enemies[i].errorTimer > 0.0f) ? (Color){ 255, 60, 60, 240 } : (Color){ 60, 255, 140, 240 };
                for (int sx = -1; sx <= 1; sx += 2)
                {
                    for (int sy = -1; sy <= 1; sy += 2)
                    {
                        float cx = drawX + sx * pad + shakeX;
                        float cy = drawY + sy * pad;
                        DrawLine((int)cx, (int)cy, (int)(cx - sx * bLen), (int)cy, reticleColor);
                        DrawLine((int)cx, (int)cy, (int)cx, (int)(cy - sy * bLen), reticleColor);
                    }
                }
            }

            const char* remainingText = &enemies[i].word[enemies[i].typedLetters];
            int textW = MeasureText(remainingText, 20);
            int badgeW = (textW + 20 > 70) ? textW + 20 : 70;

            bool hasErr = (enemies[i].errorTimer > 0.0f);
            Color badgeBg = hasErr ? (Color){ 70, 15, 25, 245 } : (Color){ 16, 20, 32, 235 };
            Color badgeBorder = hasErr ? (Color){ 255, 50, 50, 255 } : ((i == targetEnemy) ? (Color){ 60, 255, 140, 220 } : (Color){ 50, 65, 85, 200 });
            Color textColor = hasErr ? (Color){ 255, 50, 50, 255 } : ((i == targetEnemy) ? (Color){ 80, 255, 120, 255 } : WHITE);

            DrawRectangle((int)badgeDrawX - badgeW / 2, (int)badgeY, badgeW, 22, badgeBg);
            DrawRectangleLinesEx((Rectangle){ (int)badgeDrawX - badgeW / 2, (int)badgeY, (float)badgeW, 22.0f }, hasErr ? 2.0f : 1.0f, badgeBorder);
            DrawText(remainingText, (int)badgeDrawX - textW / 2, (int)badgeY + 1, 20, textColor);
        }
    }
}
