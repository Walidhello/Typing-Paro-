#include "game.h"
#include "enemy.h"
#include "player.h"
#include "word.h"
#include "gunship.h"
#include "shooting.h"
#include "leaderboard.h"
#include "media.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

GameState gameState = STATE_MENU;
char pilotName[24] = "PILOT";
int currentScore = 0;
int currentCombo = 1;
int maxCombo = 1;
int currentLevel = 1;
int wordsClearedThisLevel = 0;
int totalChars = 0;
int correctChars = 0;
float spawnTimer = 0.0f;
float spawnDelay = 2.5f;
bool isPaused = false;
int sonicWavesRemaining = MAX_SONIC_WAVES;

static int lastPlayerRank = -1;
static bool nameInputActive = true;
static float cursorTimer = 0.0f;
static GameState leaderboardReturnState = STATE_MENU;
static float launchAnimTimer = 0.0f;
static float levelClearTimer = 0.0f;
static const float LAUNCH_ANIM_DURATION = 2.4f;
static const float LEVEL_CLEAR_DURATION = 4.0f;

static const Rectangle BTN_MENU_LAUNCH = { SCREEN_WIDTH / 2.0f - 150.0f, 410.0f, 300.0f, 50.0f };
static const Rectangle BTN_MENU_BOARD  = { SCREEN_WIDTH / 2.0f - 150.0f, 474.0f, 300.0f, 44.0f };
static const Rectangle BOX_MENU_INPUT  = { SCREEN_WIDTH / 2.0f - 170.0f, 320.0f, 340.0f, 48.0f };
static const Rectangle BTN_HUD_SONIC   = { 215.0f, 14.0f, 175.0f, 82.0f };
static const Rectangle BTN_HUD_PAUSE   = { SCREEN_WIDTH - 88.0f, 60.0f, 38.0f, 34.0f };
static const Rectangle BTN_MENU_MUTE   = { SCREEN_WIDTH - 165.0f, 20.0f, 145.0f, 36.0f };
static const Rectangle BTN_CLEAR_NEXT  = { SCREEN_WIDTH / 2.0f - 170.0f, 620.0f, 340.0f, 48.0f };

static const Rectangle BTN_PAUSE_RESUME  = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f - 85.0f, 260.0f, 40.0f };
static const Rectangle BTN_PAUSE_MUTE    = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f - 35.0f, 260.0f, 40.0f };
static const Rectangle BTN_PAUSE_RESTART = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f + 15.0f, 260.0f, 40.0f };
static const Rectangle BTN_PAUSE_MENU    = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f + 65.0f, 260.0f, 40.0f };

static const Rectangle BTN_OVER_PLAY  = { 100.0f, 655.0f, 180.0f, 46.0f };
static const Rectangle BTN_OVER_BOARD = { 300.0f, 655.0f, 200.0f, 46.0f };
static const Rectangle BTN_OVER_MENU  = { 520.0f, 655.0f, 180.0f, 46.0f };
static const Rectangle BTN_BOARD_BACK = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT - 95.0f, 260.0f, 42.0f };

static int GetTargetWPM(int level) { return 20 + (level - 1) * 5; }

#define WPM_SAMPLE_WINDOW 4.0f
#define MAX_WPM_SAMPLES 256

static float charTimestamps[MAX_WPM_SAMPLES];
static int charTimestampHead = 0;
static int charTimestampCount = 0;
static float gamePlayTime = 0.0f;
static float smoothLiveWpm = 0.0f;

void RecordCorrectChar(void)
{
    correctChars++;
    charTimestamps[charTimestampHead] = gamePlayTime;
    charTimestampHead = (charTimestampHead + 1) % MAX_WPM_SAMPLES;
    if (charTimestampCount < MAX_WPM_SAMPLES) charTimestampCount++;
}

int GetAverageWPM(void)
{
    if (gamePlayTime < 1.0f || correctChars <= 0) return 0;
    return (int)roundf((correctChars / 5.0f) / (gamePlayTime / 60.0f));
}

int GetLiveWPM(void) { return (int)roundf(smoothLiveWpm); }

static void UpdateLiveWpm(float dt)
{
    int count = 0;
    for (int i = 0; i < charTimestampCount; i++)
    {
        int idx = (charTimestampHead - 1 - i + MAX_WPM_SAMPLES) % MAX_WPM_SAMPLES;
        if (gamePlayTime - charTimestamps[idx] <= WPM_SAMPLE_WINDOW) count++;
        else break;
    }

    float window = fminf(gamePlayTime, WPM_SAMPLE_WINDOW);
    float rawWpm = (window > 0.5f) ? ((count / 5.0f) / (window / 60.0f)) : 0.0f;
    // Smooth transition for live WPM gauge
    smoothLiveWpm += (rawWpm - smoothLiveWpm) * (1.0f - expf(-3.5f * dt));
    if (smoothLiveWpm < 0.0f) smoothLiveWpm = 0.0f;
}

float GetAccuracy(void)
{
    return (totalChars > 0) ? (((float)correctChars / (float)totalChars) * 100.0f) : 100.0f;
}

bool TriggerSonicWave(void)
{
    if (gameState != STATE_PLAYING || isPaused) return false;

    if (sonicWavesRemaining <= 0)
    {
        AddFloatText((Vector2){ gunship.position.x, gunship.position.y - 45.0f }, "NO CHARGES REMAINING!", (Color){ 255, 90, 90, 255 }, 20);
        return false;
    }

    sonicWavesRemaining--;
    TriggerSonicWaveVFX(gunship.position);

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            enemies[i].active = false;
            CreateExplosion((Vector2){ enemies[i].x, enemies[i].y }, enemies[i].type, "SONIC PULSE");
            currentScore += 120 * currentLevel * currentCombo;
            wordsClearedThisLevel++;
        }
    }
    targetEnemy = -1;
    CheckLevelProgression();
    return true;
}

void CheckLevelProgression(void)
{
    if (wordsClearedThisLevel >= 10)
    {
        spawnTimer = 0.0f;
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (enemies[i].active)
            {
                CreateHitSparks((Vector2){ enemies[i].x, enemies[i].y }, (Color){ 0, 240, 255, 255 }, 12);
                enemies[i].active = false;
            }
        }
        targetEnemy = -1;
        if (sonicWavesRemaining < MAX_SONIC_WAVES) sonicWavesRemaining++;
        levelClearTimer = LEVEL_CLEAR_DURATION;
        gameState = STATE_LEVEL_CLEAR;
    }
}

void SkipLevel(void)
{
    wordsClearedThisLevel = 10;
    CheckLevelProgression();
}

void StartNewGame(void)
{
    if (pilotName[0] == '\0') strncpy(pilotName, "PILOT", sizeof(pilotName) - 1);
    currentLevel = 1;
    wordsClearedThisLevel = 0;
    spawnDelay = 60.0f / (float)GetTargetWPM(1) * 0.8f;
    totalChars = 0;
    correctChars = 0;
    currentScore = 0;
    currentCombo = 1;
    maxCombo = 1;
    isPaused = false;
    spawnTimer = 0.0f;
    lastPlayerRank = -1;
    sonicWavesRemaining = MAX_SONIC_WAVES;
    gamePlayTime = 0.0f;
    charTimestampHead = 0;
    charTimestampCount = 0;
    smoothLiveWpm = 0.0f;

    InitEnemies();
    InitShooting();
    LoadWords("assets/words/easy.txt");
    SpawnEnemy();
    gameState = STATE_PLAYING;
}

void InitGame(void)
{
    InitLeaderboard();
    InitMedia();
    InitGunship();
    InitPlayer();
    LoadWords("assets/words/easy.txt");
}

static void UpdateMenu(void)
{
    float dt = GetFrameTime();
    cursorTimer += dt;
    if (cursorTimer > 1.0f) cursorTimer = 0.0f;

    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, BTN_MENU_LAUNCH) ||
                 CheckCollisionPointRec(mouse, BTN_MENU_BOARD)  ||
                 CheckCollisionPointRec(mouse, BTN_MENU_MUTE)   ||
                 CheckCollisionPointRec(mouse, BOX_MENU_INPUT);
    SetMouseCursor(hover ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    if (nameInputActive)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            int len = (int)strlen(pilotName);
            if (key >= 32 && key <= 126 && len < 16)
            {
                pilotName[len] = (char)toupper(key);
                pilotName[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            int len = (int)strlen(pilotName);
            if (len > 0) pilotName[len - 1] = '\0';
        }
    }

    if (IsKeyPressed(KEY_ENTER) || (CheckCollisionPointRec(mouse, BTN_MENU_LAUNCH) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        if (pilotName[0] == '\0') strncpy(pilotName, "PILOT", sizeof(pilotName) - 1);
        launchAnimTimer = 0.0f;
        gameState = STATE_LAUNCH_ANIMATION;
        return;
    }

    if (IsKeyPressed(KEY_TAB) || (CheckCollisionPointRec(mouse, BTN_MENU_BOARD) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        leaderboardReturnState = STATE_MENU;
        gameState = STATE_LEADERBOARD;
        return;
    }

    if (CheckCollisionPointRec(mouse, BTN_MENU_MUTE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        ToggleMusicMute();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        nameInputActive = CheckCollisionPointRec(mouse, BOX_MENU_INPUT);
}

static void UpdateLaunchAnimation(void)
{
    launchAnimTimer += GetFrameTime();
    if (launchAnimTimer >= LAUNCH_ANIM_DURATION || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        StartNewGame();
        gameState = STATE_PLAYING;
    }
}

static void UpdatePlaying(void)
{
    Vector2 mouse = GetMousePosition();

    if (!isPaused)
    {
        bool hover = CheckCollisionPointRec(mouse, BTN_HUD_SONIC) || CheckCollisionPointRec(mouse, BTN_HUD_PAUSE);
        SetMouseCursor(hover ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
            (CheckCollisionPointRec(mouse, BTN_HUD_PAUSE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        {
            isPaused = true;
            return;
        }

        if (CheckCollisionPointRec(mouse, BTN_HUD_SONIC) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            TriggerSonicWave();
    }
    else
    {
        bool hover = CheckCollisionPointRec(mouse, BTN_PAUSE_RESUME)  ||
                     CheckCollisionPointRec(mouse, BTN_PAUSE_MUTE)    ||
                     CheckCollisionPointRec(mouse, BTN_PAUSE_RESTART) ||
                     CheckCollisionPointRec(mouse, BTN_PAUSE_MENU);
        SetMouseCursor(hover ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

        if (IsKeyPressed(KEY_ESCAPE) || (CheckCollisionPointRec(mouse, BTN_PAUSE_RESUME) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        {
            isPaused = false;
            return;
        }
        if (CheckCollisionPointRec(mouse, BTN_PAUSE_MUTE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ToggleMusicMute();
            return;
        }
        if (CheckCollisionPointRec(mouse, BTN_PAUSE_RESTART) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isPaused = false;
            StartNewGame();
            return;
        }
        if (IsKeyPressed(KEY_M) || (CheckCollisionPointRec(mouse, BTN_PAUSE_MENU) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        {
            isPaused = false;
            gameState = STATE_MENU;
            return;
        }
        return;
    }

    float dt = GetFrameTime();
    gamePlayTime += dt;
    UpdateLiveWpm(dt);

    spawnTimer += dt;
    if (spawnTimer >= spawnDelay)
    {
        SpawnEnemy();
        spawnTimer = 0.0f;
    }

    UpdateEnemies();
    ProcessTyping();
    UpdateGunship();
    UpdateShooting();

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active && enemies[i].y >= (player.position.y - player.height / 2))
        {
            lastPlayerRank = AddLeaderboardEntry(pilotName, currentScore, currentLevel, GetAccuracy(), GetAverageWPM());
            gameState = STATE_GAMEOVER;
            return;
        }
    }
}

static void UpdateLevelClear(void)
{
    levelClearTimer -= GetFrameTime();
    Vector2 mouse = GetMousePosition();
    SetMouseCursor(CheckCollisionPointRec(mouse, BTN_CLEAR_NEXT) ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    if (levelClearTimer <= 0.0f || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
        (CheckCollisionPointRec(mouse, BTN_CLEAR_NEXT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        if (currentLevel < 10)
        {
            currentLevel++;
            wordsClearedThisLevel = 0;
            spawnDelay = 60.0f / (float)GetTargetWPM(currentLevel) * 0.8f;
            spawnTimer = 0.0f;
            InitEnemies();
            InitShooting();
            gameState = STATE_PLAYING;
            SpawnEnemy();
        }
        else
        {
            lastPlayerRank = AddLeaderboardEntry(pilotName, currentScore, currentLevel, GetAccuracy(), GetAverageWPM());
            gameState = STATE_GAMEOVER;
        }
    }
}

static void UpdateGameOver(void)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, BTN_OVER_PLAY) ||
                 CheckCollisionPointRec(mouse, BTN_OVER_BOARD) ||
                 CheckCollisionPointRec(mouse, BTN_OVER_MENU);
    SetMouseCursor(hover ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    if (IsKeyPressed(KEY_ENTER) || (CheckCollisionPointRec(mouse, BTN_OVER_PLAY) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        StartNewGame();
        return;
    }
    if (IsKeyPressed(KEY_TAB) || (CheckCollisionPointRec(mouse, BTN_OVER_BOARD) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        leaderboardReturnState = STATE_GAMEOVER;
        gameState = STATE_LEADERBOARD;
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE) || (CheckCollisionPointRec(mouse, BTN_OVER_MENU) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        gameState = STATE_MENU;
        return;
    }
}

static void UpdateLeaderboardScreen(void)
{
    Vector2 mouse = GetMousePosition();
    SetMouseCursor(CheckCollisionPointRec(mouse, BTN_BOARD_BACK) ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    if (IsKeyPressed(KEY_ESCAPE) || (CheckCollisionPointRec(mouse, BTN_BOARD_BACK) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        gameState = leaderboardReturnState;
    }
}

void UpdateGame(void)
{
    UpdateMedia();
    switch (gameState)
    {
        case STATE_MENU:             UpdateMenu(); break;
        case STATE_LAUNCH_ANIMATION: UpdateLaunchAnimation(); break;
        case STATE_PLAYING:          UpdatePlaying(); break;
        case STATE_LEVEL_CLEAR:      UpdateLevelClear(); break;
        case STATE_GAMEOVER:         UpdateGameOver(); break;
        case STATE_LEADERBOARD:      UpdateLeaderboardScreen(); break;
    }
}

static void DrawCenteredText(const char* text, int cx, int y, int fontSize, Color color)
{
    DrawText(text, cx - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

static void DrawCard(int x, int y, int w, int h, const char* label, const char* val, Color valCol)
{
    DrawRectangle(x, y, w, h, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)x, (float)y, (float)w, (float)h }, 1.5f, (Color){ 40, 85, 150, 255 });
    DrawCenteredText(label, x + w / 2, y + 12, 12, (Color){ 130, 165, 205, 255 });
    DrawCenteredText(val, x + w / 2, y + 36, 20, valCol);
}

static void DrawPanelCorners(int x, int y, int w, int h, int sz, Color c)
{
    DrawRectangle(x - 4, y - 4, sz, sz, c);
    DrawRectangle(x + w - sz + 4, y - 4, sz, sz, c);
    DrawRectangle(x - 4, y + h - sz + 4, sz, sz, c);
    DrawRectangle(x + w - sz + 4, y + h - sz + 4, sz, sz, c);
}

static void DrawMenuButton(Rectangle btn, const char* text, int fontSize, Color baseCol, Color hoverCol, Color borderCol, Color textCol, Color hoverTextCol)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), btn);
    DrawRectangleRec(btn, hover ? hoverCol : baseCol);
    DrawRectangleLinesEx(btn, 1.5f, hover ? WHITE : borderCol);
    DrawCenteredText(text, (int)(btn.x + btn.width / 2), (int)btn.y + ((int)btn.height - fontSize) / 2, fontSize, hover ? hoverTextCol : textCol);
}

static void DrawStartingPage(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    bool muted = IsMusicMuted();
    Color mBg = muted ? (Color){ 45, 16, 24, 230 } : (Color){ 16, 35, 65, 230 };
    Color mHov = muted ? (Color){ 70, 25, 35, 255 } : (Color){ 0, 180, 220, 255 };
    Color mBorder = muted ? (Color){ 255, 60, 80, 220 } : (Color){ 0, 230, 255, 220 };
    Color mTxtCol = muted ? (Color){ 255, 120, 140, 255 } : (Color){ 180, 230, 255, 255 };
    DrawMenuButton(BTN_MENU_MUTE, muted ? "MUSIC: MUTED" : "MUSIC: ON", 14, mBg, mHov, mBorder, mTxtCol, BLACK);

    float time = (float)GetTime();
    float origY = gunship.position.y;
    gunship.position.y = SCREEN_HEIGHT - 130.0f + sinf(time * 2.5f) * 4.0f;
    DrawGunship();
    gunship.position.y = origY;

    float scanY = 190.0f + fmodf(time * 110.0f, 400.0f);
    DrawLineEx((Vector2){ SCREEN_WIDTH / 2.0f - 240.0f, scanY }, (Vector2){ SCREEN_WIDTH / 2.0f + 240.0f, scanY }, 1.5f, Fade((Color){ 0, 240, 255, 255 }, 0.25f));

    DrawCenteredText("TYPING PARO", SCREEN_WIDTH / 2 + 3, 103, 56, (Color){ 0, 60, 160, 120 });
    DrawCenteredText("TYPING PARO", SCREEN_WIDTH / 2, 101, 56, (Color){ 0, 200, 255, 180 });
    DrawCenteredText("TYPING PARO", SCREEN_WIDTH / 2, 100, 56, WHITE);
    DrawCenteredText("TACTICAL NEURAL SPACE INTERCEPTOR", SCREEN_WIDTH / 2, 165, 16, (Color){ 100, 220, 255, 220 });
    DrawLineEx((Vector2){ SCREEN_WIDTH / 2.0f - 220.0f, 192.0f }, (Vector2){ SCREEN_WIDTH / 2.0f + 220.0f, 192.0f }, 2.0f, (Color){ 40, 120, 200, 180 });

    int boxW = 520, boxH = 380;
    int boxX = SCREEN_WIDTH / 2 - boxW / 2, boxY = 215;
    DrawRectangle(boxX, boxY, boxW, boxH, (Color){ 10, 14, 25, 235 });
    DrawRectangleLinesEx((Rectangle){ (float)boxX, (float)boxY, (float)boxW, (float)boxH }, 2.0f, (Color){ 30, 140, 230, 200 });
    DrawPanelCorners(boxX, boxY, boxW, boxH, 10, (Color){ 0, 240, 255, 255 });

    DrawCenteredText("PILOT IDENTIFICATION LINK", SCREEN_WIDTH / 2, boxY + 20, 17, (Color){ 0, 230, 255, 255 });
    DrawCenteredText("ENTER YOUR CALLSIGN TO ENGAGE:", SCREEN_WIDTH / 2, boxY + 54, 14, (Color){ 160, 190, 220, 220 });

    bool inputHover = CheckCollisionPointRec(GetMousePosition(), BOX_MENU_INPUT);
    DrawRectangleRec(BOX_MENU_INPUT, (Color){ 6, 8, 16, 255 });
    DrawRectangleLinesEx(BOX_MENU_INPUT, nameInputActive ? 2.0f : 1.0f,
                         nameInputActive ? (Color){ 0, 240, 255, 255 } : (inputHover ? (Color){ 0, 180, 220, 200 } : DARKGRAY));

    char displayName[32];
    if (pilotName[0] == '\0' && !nameInputActive)
    {
        DrawText("CALLSIGN", (int)BOX_MENU_INPUT.x + 20, (int)BOX_MENU_INPUT.y + 13, 22, (Color){ 80, 90, 110, 255 });
    }
    else
    {
        snprintf(displayName, sizeof(displayName), "%s%s", pilotName, (cursorTimer < 0.5f) ? "_" : " ");
        DrawCenteredText(displayName, SCREEN_WIDTH / 2, (int)BOX_MENU_INPUT.y + 13, 22, (Color){ 50, 255, 140, 255 });
    }

    bool launchHover = CheckCollisionPointRec(GetMousePosition(), BTN_MENU_LAUNCH);
    DrawMenuButton(BTN_MENU_LAUNCH, launchHover ? ">>> LAUNCH MISSION [ENTER] >>>" : "LAUNCH MISSION [ENTER]", 17,
                   (Color){ 16, 45, 80, 255 }, (Color){ 0, 190, 240, 255 }, (Color){ 0, 220, 255, 255 }, WHITE, BLACK);
    DrawMenuButton(BTN_MENU_BOARD, "★ HALL OF FAME [TAB] ★", 16,
                   (Color){ 14, 25, 45, 255 }, (Color){ 35, 75, 125, 255 }, (Color){ 40, 100, 160, 200 }, (Color){ 180, 220, 255, 255 }, (Color){ 180, 220, 255, 255 });

    DrawRectangle(boxX + 25, boxY + 322, boxW - 50, 36, (Color){ 16, 24, 40, 220 });
    DrawRectangleLinesEx((Rectangle){ (float)(boxX + 25), (float)(boxY + 322), (float)(boxW - 50), 36.0f }, 1.0f, (Color){ 0, 200, 255, 160 });
    DrawCenteredText("⚡ SUPERWEAPON: PRESS [SPACE] FOR SONIC WAVE (3 CHARGES)", SCREEN_WIDTH / 2, boxY + 333, 13, (Color){ 255, 220, 80, 255 });

    DrawRectangle(0, SCREEN_HEIGHT - 42, SCREEN_WIDTH, 42, (Color){ 10, 12, 20, 245 });
    DrawLine(0, SCREEN_HEIGHT - 42, SCREEN_WIDTH, SCREEN_HEIGHT - 42, (Color){ 30, 50, 80, 200 });

    char statusBuf[64];
    snprintf(statusBuf, sizeof(statusBuf), HasCustomBackground() ? "BG: Custom [assets/background.png]" : "BG: Procedural Deep Space Starfield");
    DrawText(statusBuf, 20, SCREEN_HEIGHT - 28, 13, (Color){ 120, 160, 200, 220 });

    snprintf(statusBuf, sizeof(statusBuf), HasCustomMusic() ? (muted ? "MUSIC: Muted" : "MUSIC: Active") : "MUSIC: Drop assets/music.mp3 to enable");
    DrawText(statusBuf, SCREEN_WIDTH - MeasureText(statusBuf, 13) - 20, SCREEN_HEIGHT - 28, 13, (Color){ 120, 160, 200, 220 });
}

static void DrawLaunchAnimationScreen(void)
{
    float t = launchAnimTimer;
    float warpFactor = (t > 0.7f && t < 1.9f) ? ((t - 0.7f) / 1.2f) : 0.0f;
    DrawCosmicStarfield(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f + warpFactor * 14.0f);

    if (t > 0.7f && t < 1.9f)
    {
        for (int i = 0; i < 35; i++)
        {
            float rx = (float)((i * 87) % SCREEN_WIDTH);
            float ry = fmodf((t * 2200.0f + i * 93.0f), (float)SCREEN_HEIGHT);
            float len = 60.0f + (float)(i % 5) * 35.0f;
            DrawLineEx((Vector2){ rx, ry - len }, (Vector2){ rx, ry }, 2.0f, (i % 2 == 0) ? (Color){ 0, 240, 255, 180 } : (Color){ 180, 220, 255, 220 });
        }
    }

    float shipY = SCREEN_HEIGHT - 130.0f;
    if (t < 0.7f)
    {
        gunship.flameBoost = 15.0f * (t / 0.7f);
        gunship.position.y = shipY + sinf(t * 30.0f) * 2.0f;
    }
    else if (t < 1.9f)
    {
        float p = (t - 0.7f) / 1.2f;
        gunship.flameBoost = 45.0f;
        gunship.position.y = shipY - (p * p * 520.0f);
    }
    else
    {
        float p = (t - 1.9f) / 0.5f;
        gunship.flameBoost = 15.0f * (1.0f - p);
        gunship.position.y = (SCREEN_HEIGHT - 90.0f) + (1.0f - p) * 25.0f;
    }

    DrawGunship();
    gunship.position.y = SCREEN_HEIGHT - 90.0f;

    if (t < 0.7f)
    {
        int boxW = 460, boxH = 140;
        DrawRectangle(SCREEN_WIDTH / 2 - boxW / 2, 220, boxW, boxH, (Color){ 10, 14, 25, 230 });
        DrawRectangleLinesEx((Rectangle){ (float)(SCREEN_WIDTH / 2 - boxW / 2), 220.0f, (float)boxW, (float)boxH }, 2.0f, (Color){ 0, 240, 255, 220 });

        DrawCenteredText("SYSTEM INITIALIZATION", SCREEN_WIDTH / 2, 238, 18, (Color){ 0, 240, 255, 255 });
        char pInfo[64];
        snprintf(pInfo, sizeof(pInfo), "PILOT CALLSIGN: [%s] LINKED", pilotName);
        DrawCenteredText(pInfo, SCREEN_WIDTH / 2, 270, 15, (Color){ 50, 255, 140, 255 });
        DrawCenteredText("NEURAL INTERFACE: SYNCHRONIZED 100%", SCREEN_WIDTH / 2, 296, 14, (Color){ 180, 220, 255, 220 });
        DrawCenteredText("SONIC DISRUPTORS: 3 CHARGES ARMED", SCREEN_WIDTH / 2, 318, 14, (Color){ 255, 215, 0, 255 });
    }
    else if (t < 1.9f)
    {
        DrawCenteredText("⚡ HYPERSPACE WARP ENGAGED ⚡", SCREEN_WIDTH / 2, 240, 28, (Color){ 0, 240, 255, 255 });
        DrawCenteredText("COURSE: SECTOR 01 // INTERCEPTING HOSTILE FLEET", SCREEN_WIDTH / 2, 285, 16, WHITE);
        char speedStr[40];
        snprintf(speedStr, sizeof(speedStr), "VELOCITY: WARP %.1f C", 1.0f + warpFactor * 8.9f);
        DrawCenteredText(speedStr, SCREEN_WIDTH / 2, 315, 18, (Color){ 255, 215, 0, 255 });
    }
    else
    {
        float flashAlpha = 1.0f - ((t - 1.9f) / 0.5f);
        if (flashAlpha > 0.0f)
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade((Color){ 0, 240, 255, 255 }, flashAlpha * 0.45f));

        DrawCenteredText("WARP ARRIVAL: SECTOR 01", SCREEN_WIDTH / 2, 260, 32, (Color){ 255, 60, 60, 255 });
        DrawCenteredText("ARMADA ENGAGED // ALL WEAPONS FREE!", SCREEN_WIDTH / 2, 305, 18, WHITE);
    }

    DrawCenteredText("[SPACE / ENTER / CLICK TO SKIP]", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 35, 13, (Color){ 140, 180, 220, 180 });
}

static void DrawPlayScreen(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    Camera2D camera = { .offset = GetScreenShakeOffset(), .zoom = 1.0f };
    BeginMode2D(camera);

    for (int y = 170; y < SCREEN_HEIGHT; y += 80)
        DrawLine(0, y, SCREEN_WIDTH, y, (Color){ 20, 25, 40, 160 });
    for (int x = 100; x < SCREEN_WIDTH; x += 100)
        DrawLine(x, 120, x, SCREEN_HEIGHT, (Color){ 20, 25, 40, 160 });

    DrawShootingProjectiles();
    DrawGunship();
    DrawEnemies();
    DrawShootingEffects();
    EndMode2D();

    DrawRectangle(0, 0, SCREEN_WIDTH, 112, (Color){ 10, 14, 24, 245 });
    DrawLineEx((Vector2){ 0, 112 }, (Vector2){ (float)SCREEN_WIDTH, 112 }, 2.0f, (Color){ 0, 190, 240, 220 });
    DrawLineEx((Vector2){ 0, 113 }, (Vector2){ 80, 113 }, 2.0f, (Color){ 0, 240, 255, 255 });
    DrawLineEx((Vector2){ (float)(SCREEN_WIDTH - 80), 113 }, (Vector2){ (float)SCREEN_WIDTH, 113 }, 2.0f, (Color){ 0, 240, 255, 255 });

    DrawRectangle(14, 14, 185, 82, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 14, 14, 185, 82 }, 1.5f, (Color){ 40, 80, 140, 255 });

    char buf[64];
    snprintf(buf, sizeof(buf), "PILOT: %s", pilotName);
    DrawText(buf, 24, 24, 15, (Color){ 50, 255, 140, 255 });
    snprintf(buf, sizeof(buf), "SECTOR %02d/10", currentLevel);
    DrawText(buf, 24, 46, 14, (Color){ 0, 220, 255, 255 });

    int cleared = (wordsClearedThisLevel > 10) ? 10 : wordsClearedThisLevel;
    for (int p = 0; p < 10; p++)
    {
        DrawRectangle(24 + p * 16, 72, 12, 8, (p < cleared) ? (Color){ 0, 240, 255, 255 } : (Color){ 30, 45, 70, 255 });
        DrawRectangleLines(24 + p * 16, 72, 12, 8, (Color){ 50, 80, 120, 200 });
    }

    Vector2 mouse = GetMousePosition();
    bool sonicHover = CheckCollisionPointRec(mouse, BTN_HUD_SONIC);
    Color sBg = (sonicWavesRemaining > 0) ? (sonicHover ? (Color){ 25, 55, 90, 255 } : (Color){ 16, 26, 46, 255 }) : (Color){ 22, 16, 18, 255 };
    Color sBorder = (sonicWavesRemaining > 0) ? (sonicHover ? (Color){ 255, 240, 100, 255 } : (Color){ 0, 220, 255, 220 }) : (Color){ 70, 40, 40, 200 };

    DrawRectangleRec(BTN_HUD_SONIC, sBg);
    DrawRectangleLinesEx(BTN_HUD_SONIC, 1.5f, sBorder);
    DrawText("⚡ SONIC WAVE", (int)BTN_HUD_SONIC.x + 12, (int)BTN_HUD_SONIC.y + 10, 14, (sonicWavesRemaining > 0) ? (Color){ 255, 220, 70, 255 } : DARKGRAY);
    DrawText("[KEY: SPACE]", (int)BTN_HUD_SONIC.x + 12, (int)BTN_HUD_SONIC.y + 28, 11, (Color){ 140, 180, 220, 220 });

    for (int c = 0; c < MAX_SONIC_WAVES; c++)
    {
        int cellX = (int)BTN_HUD_SONIC.x + 12 + c * 50, cellY = (int)BTN_HUD_SONIC.y + 46;
        if (c < sonicWavesRemaining)
        {
            DrawRectangle(cellX, cellY, 42, 24, (Color){ 0, 140, 220, 255 });
            DrawRectangleLines(cellX, cellY, 42, 24, (Color){ 0, 240, 255, 255 });
            DrawText("⚡", cellX + 13, cellY + 4, 15, WHITE);
        }
        else
        {
            DrawRectangle(cellX, cellY, 42, 24, (Color){ 12, 15, 24, 255 });
            DrawRectangleLines(cellX, cellY, 42, 24, (Color){ 50, 55, 70, 180 });
            DrawText("--", cellX + 14, cellY + 5, 14, (Color){ 70, 75, 90, 255 });
        }
    }

    DrawRectangle(405, 14, 150, 82, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 405, 14, 150, 82 }, 1.5f, (Color){ 40, 80, 140, 255 });
    DrawCenteredText("COMBAT SCORE", 480, 22, 13, (Color){ 140, 170, 210, 255 });
    snprintf(buf, sizeof(buf), "%d", currentScore);
    DrawCenteredText(buf, 480, 40, 24, (Color){ 0, 240, 255, 255 });

    if (currentCombo > 1)
    {
        Color comboCol = (currentCombo >= 5) ? (Color){ 255, 60, 80, 255 } :
                         (currentCombo >= 3) ? (Color){ 255, 215, 0, 255 } : (Color){ 50, 255, 140, 255 };
        snprintf(buf, sizeof(buf), "COMBO x%d!", currentCombo);
        DrawCenteredText(buf, 480, 68, 14, comboCol);
    }
    else
    {
        DrawCenteredText("COMBO READY", 480, 68, 12, (Color){ 70, 95, 130, 255 });
    }

    DrawRectangle(568, 14, 125, 82, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 568, 14, 125, 82 }, 1.5f, (Color){ 40, 80, 140, 255 });
    DrawCenteredText("LIVE WPM", 630, 22, 13, (Color){ 140, 170, 210, 255 });
    int liveWpm = GetLiveWPM();
    snprintf(buf, sizeof(buf), "%d", liveWpm);
    Color liveWpmCol = (liveWpm >= 80) ? (Color){ 0, 240, 255, 255 } :
                       (liveWpm >= 50) ? (Color){ 50, 255, 140, 255 } : (Color){ 200, 230, 255, 255 };
    DrawCenteredText(buf, 630, 42, 26, liveWpmCol);
    const char* wpmSub = (liveWpm >= 80) ? "SPEED: FAST" : (liveWpm >= 50 ? "SPEED: GOOD" : "SPEED: STEADY");
    DrawCenteredText(wpmSub, 630, 70, 10, (Color){ 120, 150, 190, 220 });

    DrawRectangle(704, 14, 82, 38, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 704, 14, 82, 38 }, 1.5f, (Color){ 40, 80, 140, 255 });
    snprintf(buf, sizeof(buf), "%.0f%%", GetAccuracy());
    DrawCenteredText(buf, 745, 24, 17, (Color){ 255, 220, 80, 255 });

    bool pauseHover = CheckCollisionPointRec(mouse, BTN_HUD_PAUSE);
    DrawRectangleRec(BTN_HUD_PAUSE, pauseHover ? (Color){ 0, 190, 240, 255 } : (Color){ 20, 30, 50, 220 });
    DrawRectangleLinesEx(BTN_HUD_PAUSE, 1.0f, pauseHover ? WHITE : (Color){ 0, 200, 255, 180 });
    DrawText("⏸", (int)BTN_HUD_PAUSE.x + 11, (int)BTN_HUD_PAUSE.y + 8, 16, pauseHover ? BLACK : WHITE);

    if (isPaused)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));
        int panelW = 440, panelH = 320;
        int panelX = SCREEN_WIDTH / 2 - panelW / 2, panelY = SCREEN_HEIGHT / 2 - panelH / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 12, 16, 28, 245 });
        DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 0, 220, 255, 255 });
        DrawPanelCorners(panelX, panelY, panelW, panelH, 10, (Color){ 0, 240, 255, 255 });

        DrawCenteredText("MISSION PAUSED", SCREEN_WIDTH / 2, panelY + 20, 26, (Color){ 255, 215, 0, 255 });
        DrawLine(panelX + 30, panelY + 54, panelX + panelW - 30, panelY + 54, (Color){ 40, 100, 160, 200 });

        DrawMenuButton(BTN_PAUSE_RESUME, "RESUME MISSION [ESC]", 15, (Color){ 18, 45, 80, 255 }, (Color){ 0, 190, 240, 255 }, (Color){ 0, 220, 255, 255 }, WHITE, BLACK);

        bool muted = IsMusicMuted();
        Color mBg = muted ? (Color){ 45, 16, 24, 230 } : (Color){ 16, 35, 65, 230 };
        Color mHov = muted ? (Color){ 70, 25, 35, 255 } : (Color){ 0, 180, 220, 255 };
        Color mBorder = muted ? (Color){ 255, 60, 80, 220 } : (Color){ 0, 230, 255, 220 };
        Color mTxtCol = muted ? (Color){ 255, 120, 140, 255 } : (Color){ 180, 230, 255, 255 };
        DrawMenuButton(BTN_PAUSE_MUTE, muted ? "MUSIC: MUTED" : "MUSIC: ON", 15, mBg, mHov, mBorder, mTxtCol, BLACK);

        DrawMenuButton(BTN_PAUSE_RESTART, "RESTART MISSION", 15, (Color){ 40, 30, 20, 255 }, (Color){ 255, 170, 30, 255 }, (Color){ 220, 140, 20, 200 }, WHITE, BLACK);
        DrawMenuButton(BTN_PAUSE_MENU, "RETURN TO MENU [M]", 15, (Color){ 45, 20, 25, 255 }, (Color){ 200, 40, 50, 255 }, (Color){ 180, 50, 60, 200 }, WHITE, WHITE);
    }
}

static void DrawLevelClearScreen(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    int panelW = 640, panelH = 580;
    int panelX = SCREEN_WIDTH / 2 - panelW / 2, panelY = 90;

    DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 10, 15, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 0, 230, 255, 255 });
    DrawPanelCorners(panelX, panelY, panelW, panelH, 12, (Color){ 0, 240, 255, 255 });

    DrawText("SYSTEM STATUS: SECTOR SECURED", panelX + 30, panelY + 25, 14, (Color){ 0, 240, 255, 255 });
    char buf[80];
    snprintf(buf, sizeof(buf), "★ SECTOR %02d LIBERATED! ★", currentLevel);
    DrawCenteredText(buf, SCREEN_WIDTH / 2, panelY + 50, 32, (Color){ 255, 215, 0, 255 });
    DrawLine(panelX + 30, panelY + 95, panelX + panelW - 30, panelY + 95, (Color){ 40, 100, 160, 200 });

    float acc = GetAccuracy();
    const char* gradeStr = (acc < 75.0f) ? "C" : (acc < 88.0f ? "B" : (acc < 95.0f ? "A" : "S"));
    Color gradeCol = (acc < 75.0f) ? (Color){ 255, 140, 40, 255 } :
                     (acc < 88.0f ? (Color){ 50, 255, 140, 255 } :
                     (acc < 95.0f ? (Color){ 0, 240, 255, 255 } : (Color){ 255, 215, 0, 255 }));
    const char* gradeDesc = (acc < 75.0f) ? "COMBAT SURVIVOR" :
                            (acc < 88.0f ? "TACTICAL SPECIALIST" :
                            (acc < 95.0f ? "VETERAN ACE PILOT" : "SUPREME MARKSMAN"));

    DrawCircle(SCREEN_WIDTH / 2, panelY + 160, 40.0f, (Color){ 16, 26, 44, 255 });
    DrawCircleLines(SCREEN_WIDTH / 2, panelY + 160, 40.0f, gradeCol);
    DrawCircleLines(SCREEN_WIDTH / 2, panelY + 160, 43.0f, Fade(gradeCol, 0.5f));
    DrawCenteredText(gradeStr, SCREEN_WIDTH / 2, panelY + 140, 40, gradeCol);
    DrawCenteredText(gradeDesc, SCREEN_WIDTH / 2, panelY + 210, 14, gradeCol);

    int cardY = panelY + 240, cardW = 126, cardH = 75, cardSpacing = 14, startCardX = panelX + 46;
    char aStr[32], wStr[32], scStr[32];
    snprintf(aStr, sizeof(aStr), "%.1f%%", acc);
    snprintf(wStr, sizeof(wStr), "%d WPM", GetAverageWPM());
    snprintf(scStr, sizeof(scStr), "%d", currentScore);

    DrawCard(startCardX + 0 * (cardW + cardSpacing), cardY, cardW, cardH, "HOSTILES", "10 / 10", WHITE);
    DrawCard(startCardX + 1 * (cardW + cardSpacing), cardY, cardW, cardH, "ACCURACY", aStr, (Color){ 255, 220, 80, 255 });
    DrawCard(startCardX + 2 * (cardW + cardSpacing), cardY, cardW, cardH, "AVG WPM", wStr, (Color){ 80, 255, 150, 255 });
    DrawCard(startCardX + 3 * (cardW + cardSpacing), cardY, cardW, cardH, "TOTAL SCORE", scStr, (Color){ 0, 230, 255, 255 });

    int pPowY = cardY + 95;
    DrawRectangle(panelX + 45, pPowY, panelW - 90, 44, (Color){ 20, 40, 65, 240 });
    DrawRectangleLinesEx((Rectangle){ (float)(panelX + 45), (float)pPowY, (float)(panelW - 90), 44.0f }, 1.5f, (Color){ 0, 240, 255, 255 });
    snprintf(buf, sizeof(buf), "⚡ REWARD: +1 SONIC POWER CELL RESTORED! (CHARGES: %d/3) ⚡", sonicWavesRemaining);
    DrawCenteredText(buf, SCREEN_WIDTH / 2, pPowY + 15, 14, (Color){ 255, 220, 80, 255 });

    int intelY = pPowY + 60;
    if (currentLevel < 10) snprintf(buf, sizeof(buf), "NEXT ZONE: SECTOR %02d // FLEET VELOCITY +25%%", currentLevel + 1);
    else snprintf(buf, sizeof(buf), "FINAL VICTORY: ALL 10 SECTORS SECURED!");
    DrawCenteredText(buf, SCREEN_WIDTH / 2, intelY, 14, (Color){ 160, 200, 240, 255 });

    float progress = fmaxf(0.0f, levelClearTimer / LEVEL_CLEAR_DURATION);
    int barW = panelW - 90, barY = intelY + 28;
    DrawRectangle(panelX + 45, barY, barW, 8, (Color){ 20, 30, 45, 255 });
    DrawRectangle(panelX + 45, barY, (int)(barW * progress), 8, (Color){ 0, 240, 255, 255 });

    snprintf(buf, sizeof(buf), "ENGAGE NEXT SECTOR (%.1fs) [SPACE]", fmaxf(0.0f, levelClearTimer));
    DrawMenuButton(BTN_CLEAR_NEXT, buf, 16, (Color){ 16, 50, 95, 255 }, (Color){ 0, 200, 255, 255 }, (Color){ 0, 230, 255, 255 }, WHITE, BLACK);
}

static void DrawGameOverScreen(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    int panelW = 680, panelH = 680;
    int panelX = SCREEN_WIDTH / 2 - panelW / 2, panelY = 50;

    DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 12, 16, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 220, 60, 60, 240 });
    DrawPanelCorners(panelX, panelY, panelW, panelH, 10, RED);

    bool win = (currentLevel >= 10 && wordsClearedThisLevel >= 10);
    DrawCenteredText(win ? "MISSION ACCOMPLISHED!" : "MISSION TERMINATED", SCREEN_WIDTH / 2, panelY + 25, 34, win ? (Color){ 255, 215, 0, 255 } : (Color){ 255, 60, 60, 255 });
    DrawCenteredText("COMBAT DEBRIEFING REPORT", SCREEN_WIDTH / 2, panelY + 68, 16, (Color){ 160, 190, 220, 220 });
    DrawLine(panelX + 30, panelY + 95, panelX + panelW - 30, panelY + 95, (Color){ 80, 40, 50, 200 });

    int startStatsY = panelY + 115;
    int avgWpm = GetAverageWPM();

    char buf[96];
    snprintf(buf, sizeof(buf), "CALLSIGN: %s // COMBAT PACE: %d AVG WPM", pilotName, avgWpm);
    DrawText(buf, panelX + 50, startStatsY, 19, (Color){ 50, 255, 140, 255 });

    if (lastPlayerRank > 0 && lastPlayerRank <= 10)
    {
        snprintf(buf, sizeof(buf), "★ NEW RECORD! RANK #%d IN HALL OF FAME! (%d AVG WPM) ★", lastPlayerRank, avgWpm);
        DrawRectangle(panelX + 40, startStatsY + 30, panelW - 80, 36, (Color){ 60, 50, 10, 220 });
        DrawRectangleLinesEx((Rectangle){ (float)(panelX + 40), (float)(startStatsY + 30), (float)(panelW - 80), 36.0f }, 1.5f, (Color){ 255, 215, 0, 255 });
        DrawCenteredText(buf, SCREEN_WIDTH / 2, startStatsY + 39, 16, (Color){ 255, 215, 0, 255 });
    }
    else
    {
        snprintf(buf, sizeof(buf), "RANKING: MISSION LOGGED // AVERAGE SPEED: %d WPM", avgWpm);
        DrawText(buf, panelX + 50, startStatsY + 35, 16, (Color){ 160, 180, 210, 220 });
    }

    int cardY = startStatsY + 80, cardW = 112, cardH = 75, cardSpacing = 10, startCardX = panelX + 40;
    char fScore[32], fLvl[16], fAcc[16], fWpm[16], fCombo[16];
    snprintf(fScore, sizeof(fScore), "%d", currentScore);
    snprintf(fLvl, sizeof(fLvl), "LV %d", currentLevel);
    snprintf(fAcc, sizeof(fAcc), "%.1f%%", GetAccuracy());
    snprintf(fWpm, sizeof(fWpm), "%d", avgWpm);
    snprintf(fCombo, sizeof(fCombo), "x%d", maxCombo);

    DrawCard(startCardX + 0 * (cardW + cardSpacing), cardY, cardW, cardH, "SCORE", fScore, (Color){ 0, 230, 255, 255 });
    DrawCard(startCardX + 1 * (cardW + cardSpacing), cardY, cardW, cardH, "SECTOR", fLvl, WHITE);
    DrawCard(startCardX + 2 * (cardW + cardSpacing), cardY, cardW, cardH, "ACCURACY", fAcc, (Color){ 255, 220, 80, 255 });
    DrawCard(startCardX + 3 * (cardW + cardSpacing), cardY, cardW, cardH, "AVG WPM", fWpm, (Color){ 80, 255, 150, 255 });
    DrawCard(startCardX + 4 * (cardW + cardSpacing), cardY, cardW, cardH, "MAX COMBO", fCombo, (Color){ 255, 180, 50, 255 });

    DrawLeaderboardMini(panelX + 40, cardY + 95, panelW - 80, lastPlayerRank);

    DrawMenuButton(BTN_OVER_PLAY, "PLAY AGAIN [ENTER]", 14, (Color){ 20, 50, 90, 255 }, (Color){ 0, 200, 255, 255 }, (Color){ 0, 220, 255, 255 }, WHITE, BLACK);
    DrawMenuButton(BTN_OVER_BOARD, "FULL LEADERBOARD [TAB]", 14, (Color){ 18, 30, 55, 255 }, (Color){ 0, 180, 220, 255 }, (Color){ 40, 100, 160, 200 }, WHITE, BLACK);
    DrawMenuButton(BTN_OVER_MENU, "MAIN MENU [ESC]", 14, (Color){ 40, 20, 28, 255 }, (Color){ 180, 40, 50, 255 }, (Color){ 180, 50, 60, 200 }, WHITE, WHITE);
}

void DrawGame(void)
{
    switch (gameState)
    {
        case STATE_MENU:             DrawStartingPage(); break;
        case STATE_LAUNCH_ANIMATION: DrawLaunchAnimationScreen(); break;
        case STATE_PLAYING:          DrawPlayScreen(); break;
        case STATE_LEVEL_CLEAR:      DrawLevelClearScreen(); break;
        case STATE_GAMEOVER:         DrawGameOverScreen(); break;
        case STATE_LEADERBOARD:      DrawLeaderboardScreen(lastPlayerRank); break;
    }
}

void UnloadGame(void)
{
    UnloadMedia();
    UnloadPlayer();
}
