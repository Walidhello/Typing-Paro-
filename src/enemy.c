#include "enemy.h"
#include "game.h"
#include "word.h"
#include "player.h"
#include "shooting.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

int targetEnemy = -1; 
Enemy enemies[MAX_ENEMIES];
static int comboStreak = 0;

void InitEnemies(void)
{
    targetEnemy = -1; 
    comboStreak = 0;
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
        enemies[i].vx = 0.0f;
        enemies[i].vy = 0.0f;
        enemies[i].headingToPlayer = false;
        enemies[i].pivotY = MEDIUM_ENEMY_PIVOT_Y;
        enemies[i].hitFlashTimer = 0.0f;
    }
}

void SpawnMinions(float x, float y) 
{
    int spawned = 0;
    LoadWords("assets/words/easy.txt"); 
    
    for(int i = 0; i < MAX_ENEMIES && spawned < 3; i++) 
    {
        if(!enemies[i].active) 
        {
            enemies[i].active = true;
            enemies[i].type = E_NORMAL;
            enemies[i].x = x + (spawned - 1) * 70; 
            enemies[i].y = y;
            
            int targetWPM = 20;
            enemies[i].speed = (300.0f * targetWPM) / 3600.0f;
            enemies[i].vx = 0.0f;
            enemies[i].vy = enemies[i].speed;
            
            strncpy(enemies[i].word, GetRandomWord(), WORD_LENGTH - 1);
            enemies[i].word[WORD_LENGTH - 1] = '\0';
            enemies[i].typedLetters = 0;
            enemies[i].hitFlashTimer = 0.0f;
            spawned++;
        }
    }
}

void SpawnEnemy(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active == false)
        {
            enemies[i].active = true;
            enemies[i].x = GetRandomValue(80, SCREEN_WIDTH - 80);
            enemies[i].y = -40;

            int targetWPM = 20;
            enemies[i].speed = (300.0f * targetWPM) / 3600.0f; 

            enemies[i].type = E_NORMAL;
            int pool = 1; 
            if (currentLevel >= 5) pool = 3;      
            else if (currentLevel >= 3) pool = 2; 

            int choice = GetRandomValue(1, pool);
            if (choice == 3) {
                enemies[i].type = E_HARD_BOSS;
                LoadWords("assets/words/hard.txt");
            } else if (choice == 2) {
                enemies[i].type = E_MEDIUM_BOSS;
                LoadWords("assets/words/medium.txt");
            } else {
                LoadWords("assets/words/easy.txt");
            }

            enemies[i].vx = 0.0f;
            enemies[i].vy = enemies[i].speed;
            enemies[i].headingToPlayer = false;
            enemies[i].pivotY = MEDIUM_ENEMY_PIVOT_Y;
            enemies[i].hitFlashTimer = 0.0f;

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

    // Safety validation for active target
    if (targetEnemy >= 0 && targetEnemy < MAX_ENEMIES)
    {
        if (!enemies[targetEnemy].active)
        {
            targetEnemy = -1;
        }
    }
    else
    {
        targetEnemy = -1;
    }

    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            // Hit flash timer decay
            if (enemies[i].hitFlashTimer > 0.0f)
            {
                enemies[i].hitFlashTimer -= dt;
                if (enemies[i].hitFlashTimer < 0.0f) enemies[i].hitFlashTimer = 0.0f;
            }

            if (enemies[i].type == E_MEDIUM_BOSS) 
            {
                float dx = player.position.x - enemies[i].x;
                float dy = player.position.y - enemies[i].y;
                float length = sqrtf(dx * dx + dy * dy);

                if (length > 0.0f) 
                {
                    float desired_vx = (dx / length) * enemies[i].speed;
                    float desired_vy = (dy / length) * enemies[i].speed;

                    float smoothing = 0.04f;
                    enemies[i].vx += (desired_vx - enemies[i].vx) * smoothing * simDt;
                    enemies[i].vy += (desired_vy - enemies[i].vy) * smoothing * simDt;
                }
            }

            enemies[i].x += enemies[i].vx * simDt;
            enemies[i].y += enemies[i].vy * simDt;

            // Despawn bounds
            if(enemies[i].y > SCREEN_HEIGHT + 50 || 
               enemies[i].y < -150 || 
               enemies[i].x < -150 || 
               enemies[i].x > SCREEN_WIDTH + 150)
            {
                enemies[i].active = false;
                if(targetEnemy == i) targetEnemy = -1; 
            }
        }
    }
}

void ProcessTyping(void)
{
    // Cheat: Skip level on apostrophe
    if (IsKeyPressed(KEY_APOSTROPHE))
    {
        targetEnemy = -1;
        SkipLevel();
        return;
    }

    // Safety check: Reset target if it is invalid or inactive
    if (targetEnemy >= 0 && targetEnemy < MAX_ENEMIES)
    {
        if (!enemies[targetEnemy].active)
        {
            targetEnemy = -1;
        }
    }
    else
    {
        targetEnemy = -1;
    }

    int key = GetCharPressed();
    bool anyKeyProcessed = false;
    bool anyKeyHit = false;

    while (key > 0)
    {
        anyKeyProcessed = true;
        totalChars++;

        int inputChar = tolower(key);

        if (targetEnemy == -1) 
        {
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active && tolower((unsigned char)enemies[i].word[0]) == inputChar)
                {
                    targetEnemy = i;
                    enemies[i].typedLetters = 1;
                    correctChars++;
                    anyKeyHit = true;

                    comboStreak++;
                    currentCombo = 1 + (comboStreak / 8);
                    if (currentCombo > 5) currentCombo = 5;
                    if (currentCombo > maxCombo) maxCombo = currentCombo;
                    currentScore += 15 * currentLevel * currentCombo;

                    enemies[i].hitFlashTimer = 0.12f;

                    if (enemies[i].typedLetters >= (int)strlen(enemies[i].word))
                    {
                        currentScore += 120 * currentLevel * currentCombo;
                        FireLaserAtPosition((Vector2){ enemies[i].x, enemies[i].y }, enemies[i].type, enemies[i].word, true);

                        if (enemies[i].type == E_HARD_BOSS)
                        {
                            SpawnMinions(enemies[i].x, enemies[i].y);
                        }

                        enemies[i].active = false;
                        targetEnemy = -1;
                        wordsClearedThisLevel++;
                        CheckLevelProgression();
                    }
                    else
                    {
                        FireLaserAtPosition((Vector2){ enemies[i].x, enemies[i].y }, enemies[i].type, enemies[i].word, false);
                    }
                    break; 
                }
            }
        }
        else 
        {
            Enemy* e = &enemies[targetEnemy];
            if (e->active && tolower((unsigned char)e->word[e->typedLetters]) == inputChar)
            {
                e->typedLetters++;
                correctChars++;
                anyKeyHit = true;

                comboStreak++;
                currentCombo = 1 + (comboStreak / 8);
                if (currentCombo > 5) currentCombo = 5;
                if (currentCombo > maxCombo) maxCombo = currentCombo;
                currentScore += 15 * currentLevel * currentCombo;

                e->hitFlashTimer = 0.12f;

                if (e->typedLetters >= (int)strlen(e->word))
                {
                    currentScore += 120 * currentLevel * currentCombo;
                    FireLaserAtPosition((Vector2){ e->x, e->y }, e->type, e->word, true);

                    if (e->type == E_HARD_BOSS)
                    {
                        SpawnMinions(e->x, e->y);
                    }

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

    // Misfire feedback & combo reset if an invalid key was typed
    if (anyKeyProcessed && !anyKeyHit)
    {
        comboStreak = 0;
        currentCombo = 1;
        TriggerMisfire();
    }
}

void DrawEnemies(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            Color boxColor = RED;
            if (enemies[i].type == E_MEDIUM_BOSS) boxColor = ORANGE;
            if (enemies[i].type == E_HARD_BOSS) boxColor = PURPLE;

            float drawX = enemies[i].x;
            float drawY = enemies[i].y;

            // Hit flash: bright white flash on damage
            if (enemies[i].hitFlashTimer > 0.0f)
            {
                boxColor = WHITE;
            }

            // Draw Enemy Ship / Hull Box
            DrawRectangle((int)drawX - 20, (int)drawY - 20, 40, 40, boxColor);

            // Inner hull detail
            DrawRectangleLines((int)drawX - 20, (int)drawY - 20, 40, 40, (Color){ 25, 25, 35, 200 });

            // Hit flash shield pulse
            if (enemies[i].hitFlashTimer > 0.0f)
            {
                DrawRectangleLinesEx((Rectangle){ drawX - 24, drawY - 24, 48, 48 }, 2.0f, SKYBLUE);
            }

            // Sci-fi Lock-on Target Reticle (for active target)
            if (i == targetEnemy)
            {
                Color reticleColor = (Color){ 60, 255, 140, 230 }; // Vibrant neon green
                float bLen = 8.0f;
                float pad = 26.0f;

                // Top-Left corner
                DrawLine((int)(drawX - pad), (int)(drawY - pad), (int)(drawX - pad + bLen), (int)(drawY - pad), reticleColor);
                DrawLine((int)(drawX - pad), (int)(drawY - pad), (int)(drawX - pad), (int)(drawY - pad + bLen), reticleColor);

                // Top-Right corner
                DrawLine((int)(drawX + pad), (int)(drawY - pad), (int)(drawX + pad - bLen), (int)(drawY - pad), reticleColor);
                DrawLine((int)(drawX + pad), (int)(drawY - pad), (int)(drawX + pad), (int)(drawY - pad + bLen), reticleColor);

                // Bottom-Left corner
                DrawLine((int)(drawX - pad), (int)(drawY + pad), (int)(drawX - pad + bLen), (int)(drawY + pad), reticleColor);
                DrawLine((int)(drawX - pad), (int)(drawY + pad), (int)(drawX - pad), (int)(drawY + pad - bLen), reticleColor);

                // Bottom-Right corner
                DrawLine((int)(drawX + pad), (int)(drawY + pad), (int)(drawX + pad - bLen), (int)(drawY + pad), reticleColor);
                DrawLine((int)(drawX + pad), (int)(drawY + pad), (int)(drawX + pad), (int)(drawY + pad - bLen), reticleColor);
            }

            // Word badge (showing full word with typed characters distinguished)
            const char* remainingText = &enemies[i].word[enemies[i].typedLetters];
            int textW = MeasureText(remainingText, 20);
            int badgeW = (textW + 20 > 70) ? textW + 20 : 70;

            DrawRectangle((int)drawX - badgeW / 2, (int)drawY - 45, badgeW, 22, (Color){ 20, 20, 30, 230 });
            DrawRectangleLines((int)drawX - badgeW / 2, (int)drawY - 45, badgeW, 22, DARKGRAY);

            Color textColor = (i == targetEnemy) ? (Color){ 80, 255, 120, 255 } : WHITE; 

            DrawText(
                remainingText,
                (int)drawX - textW / 2,
                (int)drawY - 44,
                20,
                textColor
            );
        }
    }
}
