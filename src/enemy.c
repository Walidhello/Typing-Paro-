#include "enemy.h"
#include "game.h"
#include "word.h"
#include "player.h"
#include "gunship.h"
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
        enemies[i].minionSpawnTimer = 0.0f;
    }
}

//--------------------------------------------------
// Spawn Minion Scout from Docking Station Bay
//--------------------------------------------------
void SpawnMinions(float x, float y) 
{
    // Count currently active enemies to prevent overcrowding the screen
    int activeCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active) activeCount++;
    }

    if (activeCount >= 10) return;

    LoadWords("assets/words/easy.txt"); 
    
    for(int i = 0; i < MAX_ENEMIES; i++) 
    {
        if(!enemies[i].active) 
        {
            enemies[i].active = true;
            enemies[i].type = E_NORMAL;

            // Alternate launching from left bay (-42) and right bay (+42)
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

            // Visual hangar bay launch sparks
            CreateHitSparks((Vector2){ bayX, y }, (Color){ 0, 240, 255, 255 }, 8);
            break;
        }
    }
}

//--------------------------------------------------
// Spawn Enemy Wave
//--------------------------------------------------
void SpawnEnemy(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active == false)
        {
            enemies[i].active = true;
            enemies[i].x = (float)GetRandomValue(90, SCREEN_WIDTH - 90);
            enemies[i].y = -40.0f;

            int targetWPM = 20 + (currentLevel - 1) * 5;
            enemies[i].speed = (300.0f * (float)targetWPM) / 3600.0f; 

            enemies[i].type = E_NORMAL;
            int pool = 1; 
            if (currentLevel >= 5) pool = 3;      
            else if (currentLevel >= 3) pool = 2; 

            int choice = GetRandomValue(1, pool);
            if (choice == 3) {
                enemies[i].type = E_HARD_BOSS;
                LoadWords("assets/words/hard.txt");
                // Hard boss moves slightly slower to allow minion combat
                enemies[i].speed *= 0.75f;
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
            enemies[i].minionSpawnTimer = 0.0f;

            strncpy(enemies[i].word, GetRandomWord(), WORD_LENGTH - 1);
            enemies[i].word[WORD_LENGTH - 1] = '\0';
            enemies[i].typedLetters = 0;
            break;
        }
    }
}

//--------------------------------------------------
// Update Enemies
//--------------------------------------------------
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

            // Medium boss fluid curving towards player
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

            // Hard Boss: Spawns a minion from its docking bays every 2 seconds
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

            // Despawn bounds
            if(enemies[i].y > SCREEN_HEIGHT + 60 || 
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

//--------------------------------------------------
// Process Typing
//--------------------------------------------------
void ProcessTyping(void)
{
    // Cheat: Skip level on apostrophe
    if (IsKeyPressed(KEY_APOSTROPHE))
    {
        targetEnemy = -1;
        SkipLevel();
        return;
    }

    // Trigger Sonic Wave on SPACE
    if (IsKeyPressed(KEY_SPACE))
    {
        TriggerSonicWave();
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
        // Space key triggers sonic wave without typing misfire penalty
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
            // Find all matching enemies and select the one closest to the player
            int bestCandidate = -1;
            float closestDistSq = 999999999.0f;
            float px = gunship.position.x;
            float py = gunship.position.y;

            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active && tolower((unsigned char)enemies[i].word[0]) == inputChar)
                {
                    float dx = enemies[i].x - px;
                    float dy = enemies[i].y - py;
                    float distSq = dx * dx + dy * dy;

                    if (distSq < closestDistSq)
                    {
                        closestDistSq = distSq;
                        bestCandidate = i;
                    }
                }
            }

            if (bestCandidate != -1)
            {
                int i = bestCandidate;
                targetEnemy = i;
                enemies[i].typedLetters = 1;
                RecordCorrectChar();
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

                    enemies[i].active = false;
                    targetEnemy = -1;
                    wordsClearedThisLevel++;
                    CheckLevelProgression();
                }
                else
                {
                    FireLaserAtPosition((Vector2){ enemies[i].x, enemies[i].y }, enemies[i].type, enemies[i].word, false);
                }
            }
        }
        else 
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

                if (e->typedLetters >= (int)strlen(e->word))
                {
                    currentScore += 120 * currentLevel * currentCombo;
                    FireLaserAtPosition((Vector2){ e->x, e->y }, e->type, e->word, true);

                    // Minions are now spawned periodically every 2s, NOT upon destruction
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

//--------------------------------------------------
// 1. Draw Small Spaceship (E_NORMAL / Scout Interceptor)
//--------------------------------------------------
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

    // Thruster flame (top, pointing up as ship moves down)
    DrawTriangle((Vector2){x - 5, y - 16}, (Vector2){x + 5, y - 16}, (Vector2){x, y - 24 - flicker}, (Color){255, 100, 20, 255});
    DrawTriangle((Vector2){x - 3, y - 16}, (Vector2){x + 3, y - 16}, (Vector2){x, y - 20 - flicker * 0.7f}, (Color){255, 220, 50, 255});
    DrawRectangle((int)x - 6, (int)y - 17, 12, 3, (Color){45, 50, 60, 255});

    // Left swept-forward wing
    DrawTriangle((Vector2){x - 4, y - 6}, (Vector2){x - 22, y + 10}, (Vector2){x - 12, y - 14}, (Color){130, 22, 35, 255});
    DrawLine((int)(x - 4), (int)(y - 6), (int)(x - 22), (int)(y + 10), (Color){255, 50, 70, 255});

    // Right swept-forward wing
    DrawTriangle((Vector2){x + 4, y - 6}, (Vector2){x + 12, y - 14}, (Vector2){x + 22, y + 10}, (Color){130, 22, 35, 255});
    DrawLine((int)(x + 4), (int)(y - 6), (int)(x + 22), (int)(y + 10), (Color){255, 50, 70, 255});

    // Wingtip cannon needles
    DrawLine((int)(x - 20), (int)(y + 4), (int)(x - 20), (int)(y + 14), (Color){200, 210, 225, 255});
    DrawLine((int)(x + 20), (int)(y + 4), (int)(x + 20), (int)(y + 14), (Color){200, 210, 225, 255});

    // Main fuselage wedge
    DrawTriangle((Vector2){x, y + 20}, (Vector2){x - 13, y - 14}, (Vector2){x + 13, y - 14}, (Color){38, 22, 28, 255});
    DrawTriangle((Vector2){x, y + 16}, (Vector2){x - 9, y - 10}, (Vector2){x + 9, y - 10}, (Color){185, 30, 45, 255});

    // Center armor ridge
    DrawLine((int)x, (int)(y - 12), (int)x, (int)(y + 14), (Color){255, 120, 140, 255});

    // Sensor cockpit eye
    DrawTriangle((Vector2){x, y + 6}, (Vector2){x - 4, y - 3}, (Vector2){x + 4, y - 3}, (Color){255, 215, 0, 255});
    DrawCircle((int)x, (int)y, 2.0f, WHITE);
}

//--------------------------------------------------
// 2. Draw Bigger Spaceship (E_MEDIUM_BOSS / Heavy Battlecruiser)
//--------------------------------------------------
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

    // Twin heavy thrusters (top, pointing up)
    // Left flame
    DrawTriangle((Vector2){x - 24, y - 24}, (Vector2){x - 14, y - 24}, (Vector2){x - 19, y - 35 - flicker}, (Color){255, 120, 20, 255});
    DrawTriangle((Vector2){x - 22, y - 24}, (Vector2){x - 16, y - 24}, (Vector2){x - 19, y - 30 - flicker * 0.7f}, (Color){255, 220, 60, 255});
    DrawRectangle((int)x - 25, (int)y - 25, 12, 4, (Color){50, 55, 65, 255});

    // Right flame
    DrawTriangle((Vector2){x + 14, y - 24}, (Vector2){x + 24, y - 24}, (Vector2){x + 19, y - 35 - flicker}, (Color){255, 120, 20, 255});
    DrawTriangle((Vector2){x + 16, y - 24}, (Vector2){x + 22, y - 24}, (Vector2){x + 19, y - 30 - flicker * 0.7f}, (Color){255, 220, 60, 255});
    DrawRectangle((int)x + 13, (int)y - 25, 12, 4, (Color){50, 55, 65, 255});

    // Left heavy sponson wing
    DrawTriangle((Vector2){x - 10, y - 6}, (Vector2){x - 38, y + 12}, (Vector2){x - 24, y - 22}, (Color){150, 85, 15, 255});
    DrawTriangle((Vector2){x - 14, y - 4}, (Vector2){x - 34, y + 8}, (Vector2){x - 22, y - 16}, (Color){215, 130, 25, 255});
    DrawLineEx((Vector2){x - 10, y - 6}, (Vector2){x - 38, y + 12}, 2.0f, (Color){255, 190, 50, 255});

    // Right heavy sponson wing
    DrawTriangle((Vector2){x + 10, y - 6}, (Vector2){x + 24, y - 22}, (Vector2){x + 38, y + 12}, (Color){150, 85, 15, 255});
    DrawTriangle((Vector2){x + 14, y - 4}, (Vector2){x + 22, y - 16}, (Vector2){x + 34, y + 8}, (Color){215, 130, 25, 255});
    DrawLineEx((Vector2){x + 10, y - 6}, (Vector2){x + 38, y + 12}, 2.0f, (Color){255, 190, 50, 255});

    // Heavy forward cannons
    DrawRectangle((int)x - 32, (int)y + 2, 6, 16, (Color){40, 45, 55, 255});
    DrawLine((int)(x - 29), (int)(y + 18), (int)(x - 29), (int)(y + 24), (Color){255, 200, 70, 255});

    DrawRectangle((int)x + 26, (int)y + 2, 6, 16, (Color){40, 45, 55, 255});
    DrawLine((int)(x + 29), (int)(y + 18), (int)(x + 29), (int)(y + 24), (Color){255, 200, 70, 255});

    // Main hull wedge
    DrawTriangle((Vector2){x, y + 28}, (Vector2){x - 26, y - 20}, (Vector2){x + 26, y - 20}, (Color){32, 35, 45, 255});
    DrawTriangle((Vector2){x, y + 22}, (Vector2){x - 18, y - 14}, (Vector2){x + 18, y - 14}, (Color){195, 115, 20, 255});

    // Command bridge visor
    DrawRectangle((int)x - 10, (int)y + 4, 20, 7, (Color){20, 25, 35, 255});
    DrawRectangle((int)x - 8, (int)y + 5, 16, 5, (Color){255, 220, 60, 255});

    // Glowing core
    DrawCircle((int)x, (int)y - 6, 7.0f, (Color){40, 50, 70, 255});
    DrawCircle((int)x, (int)y - 6, 4.5f, (Color){255, 160, 20, 255});
    DrawCircle((int)x, (int)y - 6, 2.0f, WHITE);

    // Strobe wingtip beacons
    DrawCircle((int)x - 37, (int)y + 11, 2.5f, (Color){255, 210, 50, 255});
    DrawCircle((int)x + 37, (int)y + 11, 2.5f, (Color){255, 210, 50, 255});
}

//--------------------------------------------------
// 3. Draw Space Docking Station (E_HARD_BOSS / Orbital Starbase)
//--------------------------------------------------
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

    // 1. Solar Array Panels (Top-left & Top-right)
    DrawRectangle((int)x - 44, (int)y - 38, 26, 12, (Color){18, 45, 85, 255});
    DrawRectangleLines((int)x - 44, (int)y - 38, 26, 12, (Color){0, 180, 255, 200});
    DrawLine((int)x - 31, (int)y - 38, (int)x - 31, (int)y - 26, (Color){0, 220, 255, 180});

    DrawRectangle((int)x + 18, (int)y - 38, 26, 12, (Color){18, 45, 85, 255});
    DrawRectangleLines((int)x + 18, (int)y - 38, 26, 12, (Color){0, 180, 255, 200});
    DrawLine((int)x + 31, (int)y - 38, (int)x + 31, (int)y - 26, (Color){0, 220, 255, 180});

    // Pylon trusses connecting solar arrays to central hub
    DrawLineEx((Vector2){x - 20, y - 18}, (Vector2){x - 31, y - 26}, 2.5f, (Color){60, 65, 85, 255});
    DrawLineEx((Vector2){x + 20, y - 18}, (Vector2){x + 31, y - 26}, 2.5f, (Color){60, 65, 85, 255});

    // 2. Station Backbone Spines (Horizontal and Vertical structural arms)
    DrawRectangle((int)x - 48, (int)y - 8, 96, 16, (Color){38, 32, 50, 255});
    DrawRectangle((int)x - 8, (int)y - 34, 16, 68, (Color){38, 32, 50, 255});

    // Glowing cyan power conduits along the station spine
    DrawLineEx((Vector2){x - 46, y}, (Vector2){x + 46, y}, 1.5f, (Color){0, 230, 255, 200});
    DrawLineEx((Vector2){x, y - 32}, (Vector2){x, y + 32}, 1.5f, (Color){0, 230, 255, 200});

    // 3. Left Docking/Hangar Bay (Minions launch from here)
    DrawRectangle((int)x - 52, (int)y - 16, 18, 32, (Color){26, 20, 36, 255});
    DrawRectangleLinesEx((Rectangle){x - 52, y - 16, 18, 32}, 1.5f, (Color){150, 45, 195, 255});

    // Runway / Launch Guide Lights
    DrawRectangle((int)x - 50, (int)y + 9, 14, 4, (Color){0, 240, 255, 255});
    DrawLine((int)x - 48, (int)y - 12, (int)x - 48, (int)y + 7, (Color){255, 215, 0, 220});
    DrawLine((int)x - 38, (int)y - 12, (int)x - 38, (int)y + 7, (Color){255, 215, 0, 220});

    // 4. Right Docking/Hangar Bay (Minions launch from here)
    DrawRectangle((int)x + 34, (int)y - 16, 18, 32, (Color){26, 20, 36, 255});
    DrawRectangleLinesEx((Rectangle){x + 34, y - 16, 18, 32}, 1.5f, (Color){150, 45, 195, 255});

    // Runway / Launch Guide Lights
    DrawRectangle((int)x + 36, (int)y + 9, 14, 4, (Color){0, 240, 255, 255});
    DrawLine((int)x + 38, (int)y - 12, (int)x + 38, (int)y + 7, (Color){255, 215, 0, 220});
    DrawLine((int)x + 48, (int)y - 12, (int)x + 48, (int)y + 7, (Color){255, 215, 0, 220});

    // 5. Forward Defense Antennas & Communication Towers (bottom)
    DrawLineEx((Vector2){x - 18, y + 26}, (Vector2){x - 18, y + 42}, 2.0f, (Color){180, 190, 215, 255});
    DrawLineEx((Vector2){x + 18, y + 26}, (Vector2){x + 18, y + 42}, 2.0f, (Color){180, 190, 215, 255});
    DrawCircle((int)x - 18, (int)y + 42, 2.0f, RED);
    DrawCircle((int)x + 18, (int)y + 42, 2.0f, RED);

    // 6. Central Command Citadel & Octagonal Hub
    DrawPoly((Vector2){x, y}, 8, 25.0f, 22.5f, (Color){30, 24, 42, 255});
    DrawPoly((Vector2){x, y}, 8, 20.0f, 22.5f, (Color){90, 32, 115, 255});

    // Pulsing Fusion Reactor Core
    float pulse = 0.85f + 0.15f * sinf(time * 6.0f);
    DrawCircle((int)x, (int)y, 11.0f * pulse, (Color){190, 35, 230, 255});
    DrawCircle((int)x, (int)y, 7.0f * pulse, (Color){255, 80, 240, 255});
    DrawCircle((int)x, (int)y, 3.5f, WHITE);

    // 7. Navigational Beacons (Blinking)
    bool blink = ((int)(time * 3.5f) % 2 == 0);
    DrawCircle((int)x - 52, (int)y, 3.0f, blink ? (Color){0, 240, 255, 255} : (Color){0, 70, 100, 255});
    DrawCircle((int)x + 52, (int)y, 3.0f, blink ? (Color){0, 240, 255, 255} : (Color){0, 70, 100, 255});
}

//--------------------------------------------------
// Draw All Enemies with Graphical Models & Badges
//--------------------------------------------------
void DrawEnemies(void)
{
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            float drawX = enemies[i].x;
            float drawY = enemies[i].y;
            bool isHit = (enemies[i].hitFlashTimer > 0.0f);

            float badgeY = drawY - 42.0f;
            float pad = 26.0f;
            float bLen = 8.0f;

            // 1. Draw Graphical Space Craft / Station Models
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
                badgeY = drawY - 38.0f;
                pad = 26.0f;
                bLen = 8.0f;
            }

            // 2. Sci-Fi Lock-On Target Reticle (for active target)
            if (i == targetEnemy)
            {
                Color reticleColor = (Color){ 60, 255, 140, 240 }; // Neon green

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

            // 3. Word Badge
            const char* remainingText = &enemies[i].word[enemies[i].typedLetters];
            int textW = MeasureText(remainingText, 20);
            int badgeW = (textW + 20 > 70) ? textW + 20 : 70;

            DrawRectangle((int)drawX - badgeW / 2, (int)badgeY, badgeW, 22, (Color){ 16, 20, 32, 235 });
            DrawRectangleLinesEx(
                (Rectangle){ (int)drawX - badgeW / 2, (int)badgeY, (float)badgeW, 22.0f },
                1.0f,
                (i == targetEnemy) ? (Color){ 60, 255, 140, 220 } : (Color){ 50, 65, 85, 200 }
            );

            Color textColor = (i == targetEnemy) ? (Color){ 80, 255, 120, 255 } : WHITE; 

            DrawText(
                remainingText,
                (int)drawX - textW / 2,
                (int)badgeY + 1,
                20,
                textColor
            );
        }
    }
}
