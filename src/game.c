#include "game.h"
#include "enemy.h"
#include "player.h"
#include "word.h"
#include "gunship.h"
#include "shooting.h"
#include "leaderboard.h"
#include "media.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define PI_FLOAT 3.14159265358979323846f

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

static const Rectangle BTN_HUD_SONIC = { 215.0f, 14.0f, 175.0f, 82.0f };
static const Rectangle BTN_HUD_PAUSE = { SCREEN_WIDTH - 88.0f, 60.0f, 38.0f, 34.0f };
static const Rectangle BTN_HUD_MUTE  = { SCREEN_WIDTH - 46.0f, 60.0f, 38.0f, 34.0f };

static const Rectangle BTN_CLEAR_NEXT = { SCREEN_WIDTH / 2.0f - 170.0f, 620.0f, 340.0f, 48.0f };

static const Rectangle BTN_PAUSE_RESUME  = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f - 40.0f, 260.0f, 42.0f };
static const Rectangle BTN_PAUSE_RESTART = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f + 14.0f, 260.0f, 42.0f };
static const Rectangle BTN_PAUSE_MENU    = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT / 2.0f + 68.0f, 260.0f, 42.0f };

static const Rectangle BTN_OVER_PLAY  = { 100.0f, 655.0f, 180.0f, 46.0f };
static const Rectangle BTN_OVER_BOARD = { 300.0f, 655.0f, 200.0f, 46.0f };
static const Rectangle BTN_OVER_MENU  = { 520.0f, 655.0f, 180.0f, 46.0f };

static const Rectangle BTN_BOARD_BACK = { SCREEN_WIDTH / 2.0f - 130.0f, SCREEN_HEIGHT - 95.0f, 260.0f, 42.0f };

static int GetTargetWPM(int level)
{
    return 20 + (level - 1) * 5;
}

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
    if (charTimestampCount < MAX_WPM_SAMPLES)
        charTimestampCount++;
}

int GetAverageWPM(void)
{
    if (gamePlayTime < 1.0f || correctChars <= 0)
        return 0;

    float minutes = gamePlayTime / 60.0f;
    float wpm = (correctChars / 5.0f) / minutes;
    return (int)roundf(wpm);
}

int GetLiveWPM(void)
{
    return (int)roundf(smoothLiveWpm);
}

static void UpdateLiveWpm(float dt)
{
    int count = 0;
    for (int i = 0; i < charTimestampCount; i++)
    {
        int idx = (charTimestampHead - 1 - i + MAX_WPM_SAMPLES) % MAX_WPM_SAMPLES;
        if (gamePlayTime - charTimestamps[idx] <= WPM_SAMPLE_WINDOW)
        {
            count++;
        }
        else
        {
            break;
        }
    }

    float window = fminf(gamePlayTime, WPM_SAMPLE_WINDOW);
    if (window < 1.0f) window = 1.0f;

    float instantWpm = 0.0f;
    if (count > 0)
    {
        instantWpm = (count / 5.0f) / (window / 60.0f);
    }

    // Smooth transition for live WPM gauge
    smoothLiveWpm += (instantWpm - smoothLiveWpm) * (1.0f - expf(-5.0f * dt));
    if (smoothLiveWpm < 0.5f) smoothLiveWpm = 0.0f;
}

float GetAccuracy(void)
{
    if (totalChars == 0)
        return 100.0f;

    return ((float)correctChars / (float)totalChars) * 100.0f;
}

bool TriggerSonicWave(void)
{
    if (gameState != STATE_PLAYING || isPaused)
        return false;

    if (sonicWavesRemaining <= 0)
    {
        AddFloatText(
            (Vector2){ gunship.position.x, gunship.position.y - 45.0f },
            "NO CHARGES REMAINING!",
            (Color){ 255, 90, 90, 255 },
            20
        );
        return false;
    }

    sonicWavesRemaining--;

    TriggerSonicWaveVFX(gunship.position);

    int destroyed = 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            enemies[i].active = false;
            CreateExplosion((Vector2){ enemies[i].x, enemies[i].y }, enemies[i].type, "SONIC PULSE");
            currentScore += 120 * currentLevel * currentCombo;
            destroyed++;
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

        if (sonicWavesRemaining < MAX_SONIC_WAVES)
        {
            sonicWavesRemaining++;
        }

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
    if (strlen(pilotName) == 0)
    {
        strncpy(pilotName, "PILOT", sizeof(pilotName) - 1);
    }

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
    InitMedia();
    InitLeaderboard();
    InitPlayer();
    InitGunship();
    InitShooting();

    gameState = STATE_MENU;
    strncpy(pilotName, "PILOT", sizeof(pilotName) - 1);
    nameInputActive = true;
    cursorTimer = 0.0f;
    launchAnimTimer = 0.0f;
    levelClearTimer = 0.0f;
    sonicWavesRemaining = MAX_SONIC_WAVES;
    gamePlayTime = 0.0f;
    charTimestampHead = 0;
    charTimestampCount = 0;
    smoothLiveWpm = 0.0f;
}

static void UpdateMenu(void)
{
    float dt = GetFrameTime();
    cursorTimer += dt;
    if (cursorTimer > 1.0f) cursorTimer = 0.0f;

    Vector2 mousePos = GetMousePosition();

    if (CheckCollisionPointRec(mousePos, BTN_MENU_LAUNCH) ||
        CheckCollisionPointRec(mousePos, BTN_MENU_BOARD)  ||
        CheckCollisionPointRec(mousePos, BOX_MENU_INPUT))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if (nameInputActive)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            int len = (int)strlen(pilotName);
            if ((key >= 32 && key <= 126) && len < 16)
            {
                pilotName[len] = (char)toupper(key);
                pilotName[len + 1] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            int len = (int)strlen(pilotName);
            if (len > 0)
            {
                pilotName[len - 1] = '\0';
            }
        }
    }

    bool launchTriggered = false;
    if (IsKeyPressed(KEY_ENTER))
    {
        launchTriggered = true;
    }
    if (CheckCollisionPointRec(mousePos, BTN_MENU_LAUNCH) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        launchTriggered = true;
    }

    if (launchTriggered)
    {
        if (strlen(pilotName) == 0)
        {
            strncpy(pilotName, "PILOT", sizeof(pilotName) - 1);
        }
        launchAnimTimer = 0.0f;
        gameState = STATE_LAUNCH_ANIMATION;
        return;
    }

    if (IsKeyPressed(KEY_TAB))
    {
        leaderboardReturnState = STATE_MENU;
        gameState = STATE_LEADERBOARD;
        return;
    }

    if (CheckCollisionPointRec(mousePos, BTN_MENU_BOARD) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        leaderboardReturnState = STATE_MENU;
        gameState = STATE_LEADERBOARD;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        nameInputActive = CheckCollisionPointRec(mousePos, BOX_MENU_INPUT);
    }
}

static void UpdateLaunchAnimation(void)
{
    float dt = GetFrameTime();
    launchAnimTimer += dt;

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && launchAnimTimer > 0.25f))
    {
        StartNewGame();
        return;
    }

    if (launchAnimTimer >= LAUNCH_ANIM_DURATION)
    {
        StartNewGame();
        return;
    }
}

static void UpdatePlaying(void)
{
    Vector2 mousePos = GetMousePosition();

    if (!isPaused)
    {
        if (CheckCollisionPointRec(mousePos, BTN_HUD_SONIC) ||
            CheckCollisionPointRec(mousePos, BTN_HUD_PAUSE) ||
            CheckCollisionPointRec(mousePos, BTN_HUD_MUTE))
        {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

            if (CheckCollisionPointRec(mousePos, BTN_HUD_SONIC) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                TriggerSonicWave();
                return;
            }

            if (CheckCollisionPointRec(mousePos, BTN_HUD_PAUSE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                isPaused = true;
                return;
            }

            if (CheckCollisionPointRec(mousePos, BTN_HUD_MUTE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                ToggleMusicMute();
            }
        }
        else
        {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        isPaused = !isPaused;
    }

    if (isPaused)
    {
        if (CheckCollisionPointRec(mousePos, BTN_PAUSE_RESUME) ||
            CheckCollisionPointRec(mousePos, BTN_PAUSE_RESTART) ||
            CheckCollisionPointRec(mousePos, BTN_PAUSE_MENU))
        {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        else
        {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        if (CheckCollisionPointRec(mousePos, BTN_PAUSE_RESUME) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isPaused = false;
            return;
        }

        if (CheckCollisionPointRec(mousePos, BTN_PAUSE_RESTART) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isPaused = false;
            StartNewGame();
            return;
        }

        if ((CheckCollisionPointRec(mousePos, BTN_PAUSE_MENU) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_M))
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
        if (enemies[i].active)
        {
            if (enemies[i].y >= player.position.y - (player.height / 2))
            {
                int avgWpm = GetAverageWPM();
                lastPlayerRank = AddLeaderboardEntry(pilotName, currentScore, currentLevel, GetAccuracy(), avgWpm);
                gameState = STATE_GAMEOVER;
                return;
            }
        }
    }
}

static void UpdateLevelClear(void)
{
    float dt = GetFrameTime();
    levelClearTimer -= dt;

    Vector2 mousePos = GetMousePosition();

    if (CheckCollisionPointRec(mousePos, BTN_CLEAR_NEXT))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    bool advance = false;
    if (levelClearTimer <= 0.0f ||
        IsKeyPressed(KEY_SPACE) ||
        IsKeyPressed(KEY_ENTER) ||
        (CheckCollisionPointRec(mousePos, BTN_CLEAR_NEXT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        advance = true;
    }

    if (advance)
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
            int avgWpm = GetAverageWPM();
            lastPlayerRank = AddLeaderboardEntry(pilotName, currentScore, currentLevel, GetAccuracy(), avgWpm);
            gameState = STATE_GAMEOVER;
        }
    }
}

static void UpdateGameOver(void)
{
    Vector2 mousePos = GetMousePosition();

    if (CheckCollisionPointRec(mousePos, BTN_OVER_PLAY) ||
        CheckCollisionPointRec(mousePos, BTN_OVER_BOARD) ||
        CheckCollisionPointRec(mousePos, BTN_OVER_MENU))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if (IsKeyPressed(KEY_ENTER) ||
        (CheckCollisionPointRec(mousePos, BTN_OVER_PLAY) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        StartNewGame();
        return;
    }

    if (IsKeyPressed(KEY_TAB) ||
        (CheckCollisionPointRec(mousePos, BTN_OVER_BOARD) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        leaderboardReturnState = STATE_GAMEOVER;
        gameState = STATE_LEADERBOARD;
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE) ||
        (CheckCollisionPointRec(mousePos, BTN_OVER_MENU) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        gameState = STATE_MENU;
        return;
    }
}

static void UpdateLeaderboardScreen(void)
{
    Vector2 mousePos = GetMousePosition();

    if (CheckCollisionPointRec(mousePos, BTN_BOARD_BACK))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
        (CheckCollisionPointRec(mousePos, BTN_BOARD_BACK) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
    {
        gameState = leaderboardReturnState;
        return;
    }
}

void UpdateGame(void)
{
    UpdateMedia();

    switch (gameState)
    {
        case STATE_MENU:
            UpdateMenu();
            break;

        case STATE_LAUNCH_ANIMATION:
            UpdateLaunchAnimation();
            break;

        case STATE_PLAYING:
            UpdatePlaying();
            break;

        case STATE_LEVEL_CLEAR:
            UpdateLevelClear();
            break;

        case STATE_GAMEOVER:
            UpdateGameOver();
            break;

        case STATE_LEADERBOARD:
            UpdateLeaderboardScreen();
            break;
    }
}

static void DrawStartingPage(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    float time = (float)GetTime();

    float bobbing = sinf(time * 2.5f) * 4.0f;
    float origY = gunship.position.y;
    gunship.position.y = SCREEN_HEIGHT - 130.0f + bobbing;
    DrawGunship();
    gunship.position.y = origY;

    float scanY = 190.0f + fmodf(time * 110.0f, 400.0f);
    DrawLineEx((Vector2){ SCREEN_WIDTH / 2.0f - 240.0f, scanY }, (Vector2){ SCREEN_WIDTH / 2.0f + 240.0f, scanY }, 1.5f, Fade((Color){ 0, 240, 255, 255 }, 0.25f));

    const char* title = "TYPING PARO";
    int titleSize = 56;
    int titleW = MeasureText(title, titleSize);
    int titleX = SCREEN_WIDTH / 2 - titleW / 2;
    int titleY = 100;

    DrawText(title, titleX + 3, titleY + 3, titleSize, (Color){ 0, 60, 160, 120 });
    DrawText(title, titleX, titleY + 1, titleSize, (Color){ 0, 200, 255, 180 });
    DrawText(title, titleX, titleY, titleSize, WHITE);

    const char* sub = "TACTICAL NEURAL SPACE INTERCEPTOR";
    int subW = MeasureText(sub, 16);
    DrawText(sub, SCREEN_WIDTH / 2 - subW / 2, 165, 16, (Color){ 100, 220, 255, 220 });

    DrawLineEx((Vector2){ SCREEN_WIDTH / 2.0f - 220.0f, 192.0f }, (Vector2){ SCREEN_WIDTH / 2.0f + 220.0f, 192.0f }, 2.0f, (Color){ 40, 120, 200, 180 });

    int boxW = 520;
    int boxH = 380;
    int boxX = SCREEN_WIDTH / 2 - boxW / 2;
    int boxY = 215;

    DrawRectangle(boxX, boxY, boxW, boxH, (Color){ 10, 14, 25, 235 });
    DrawRectangleLinesEx((Rectangle){ (float)boxX, (float)boxY, (float)boxW, (float)boxH }, 2.0f, (Color){ 30, 140, 230, 200 });

    DrawRectangle(boxX - 4, boxY - 4, 10, 10, (Color){ 0, 240, 255, 255 });
    DrawRectangle(boxX + boxW - 6, boxY - 4, 10, 10, (Color){ 0, 240, 255, 255 });
    DrawRectangle(boxX - 4, boxY + boxH - 6, 10, 10, (Color){ 0, 240, 255, 255 });
    DrawRectangle(boxX + boxW - 6, boxY + boxH - 6, 10, 10, (Color){ 0, 240, 255, 255 });

    const char* termHead = "PILOT IDENTIFICATION LINK";
    int termW = MeasureText(termHead, 17);
    DrawText(termHead, SCREEN_WIDTH / 2 - termW / 2, boxY + 20, 17, (Color){ 0, 230, 255, 255 });

    const char* prompt = "ENTER YOUR CALLSIGN TO ENGAGE:";
    int pW = MeasureText(prompt, 14);
    DrawText(prompt, SCREEN_WIDTH / 2 - pW / 2, boxY + 54, 14, (Color){ 160, 190, 220, 220 });

    Vector2 mouse = GetMousePosition();
    bool inputHover = CheckCollisionPointRec(mouse, BOX_MENU_INPUT);

    DrawRectangleRec(BOX_MENU_INPUT, (Color){ 6, 8, 16, 255 });
    DrawRectangleLinesEx(
        BOX_MENU_INPUT,
        nameInputActive ? 2.0f : 1.0f,
        nameInputActive ? (Color){ 0, 240, 255, 255 } : (inputHover ? (Color){ 0, 180, 220, 200 } : DARKGRAY)
    );

    char displayName[32];
    if (strlen(pilotName) == 0 && !nameInputActive)
    {
        strncpy(displayName, "CALLSIGN", sizeof(displayName));
        DrawText(displayName, (int)BOX_MENU_INPUT.x + 20, (int)BOX_MENU_INPUT.y + 13, 22, (Color){ 80, 90, 110, 255 });
    }
    else
    {
        bool showCursor = (cursorTimer < 0.5f);
        snprintf(displayName, sizeof(displayName), "%s%s", pilotName, showCursor ? "_" : " ");
        int textW = MeasureText(displayName, 22);
        DrawText(displayName, SCREEN_WIDTH / 2 - textW / 2, (int)BOX_MENU_INPUT.y + 13, 22, (Color){ 50, 255, 140, 255 });
    }

    bool launchHover = CheckCollisionPointRec(mouse, BTN_MENU_LAUNCH);
    DrawRectangleRec(BTN_MENU_LAUNCH, launchHover ? (Color){ 0, 190, 240, 255 } : (Color){ 16, 45, 80, 255 });
    DrawRectangleLinesEx(BTN_MENU_LAUNCH, 2.0f, launchHover ? WHITE : (Color){ 0, 220, 255, 255 });

    const char* launchText = launchHover ? ">>> LAUNCH MISSION [ENTER] >>>" : "LAUNCH MISSION [ENTER]";
    int lW = MeasureText(launchText, 17);
    DrawText(launchText, SCREEN_WIDTH / 2 - lW / 2, (int)BTN_MENU_LAUNCH.y + 16, 17, launchHover ? BLACK : WHITE);

    bool boardHover = CheckCollisionPointRec(mouse, BTN_MENU_BOARD);
    DrawRectangleRec(BTN_MENU_BOARD, boardHover ? (Color){ 35, 75, 125, 255 } : (Color){ 14, 25, 45, 255 });
    DrawRectangleLinesEx(BTN_MENU_BOARD, 1.5f, boardHover ? (Color){ 0, 240, 255, 255 } : (Color){ 40, 100, 160, 200 });

    const char* boardText = "★ HALL OF FAME [TAB] ★";
    int bW = MeasureText(boardText, 16);
    DrawText(boardText, SCREEN_WIDTH / 2 - bW / 2, (int)BTN_MENU_BOARD.y + 14, 16, (Color){ 180, 220, 255, 255 });

    DrawRectangle(boxX + 25, boxY + 322, boxW - 50, 36, (Color){ 16, 24, 40, 220 });
    DrawRectangleLinesEx((Rectangle){ (float)(boxX + 25), (float)(boxY + 322), (float)(boxW - 50), 36.0f }, 1.0f, (Color){ 0, 200, 255, 160 });
    const char* powerTip = "⚡ SUPERWEAPON: PRESS [SPACE] FOR SONIC WAVE (3 CHARGES)";
    int ptW = MeasureText(powerTip, 13);
    DrawText(powerTip, SCREEN_WIDTH / 2 - ptW / 2, boxY + 333, 13, (Color){ 255, 220, 80, 255 });

    DrawRectangle(0, SCREEN_HEIGHT - 42, SCREEN_WIDTH, 42, (Color){ 10, 12, 20, 245 });
    DrawLine(0, SCREEN_HEIGHT - 42, SCREEN_WIDTH, SCREEN_HEIGHT - 42, (Color){ 30, 50, 80, 200 });

    char bgStatus[64];
    if (HasCustomBackground())
        snprintf(bgStatus, sizeof(bgStatus), "BG: Custom [assets/background.png]");
    else
        snprintf(bgStatus, sizeof(bgStatus), "BG: Procedural Deep Space Starfield");
    DrawText(bgStatus, 20, SCREEN_HEIGHT - 28, 13, (Color){ 120, 160, 200, 220 });

    char musicStatus[64];
    if (HasCustomMusic())
        snprintf(musicStatus, sizeof(musicStatus), "MUSIC: %s [Click or M to Mute]", IsMusicMuted() ? "Muted" : "Active");
    else
        snprintf(musicStatus, sizeof(musicStatus), "MUSIC: Drop assets/music.mp3 to enable");
    int mLen = MeasureText(musicStatus, 13);
    DrawText(musicStatus, SCREEN_WIDTH - mLen - 20, SCREEN_HEIGHT - 28, 13, (Color){ 120, 160, 200, 220 });
}

static void DrawLaunchAnimationScreen(void)
{
    float t = launchAnimTimer;

    float speedMult = 1.0f;
    if (t > 0.7f && t < 1.9f)
    {
        float warpFactor = (t - 0.7f) / 1.2f;
        speedMult = 1.0f + warpFactor * 14.0f;
    }
    DrawCosmicStarfield(SCREEN_WIDTH, SCREEN_HEIGHT, speedMult);

    if (t > 0.7f && t < 1.9f)
    {
        for (int i = 0; i < 35; i++)
        {
            float rx = (float)((i * 87) % SCREEN_WIDTH);
            float ry = fmodf((t * 2200.0f + i * 93.0f), (float)SCREEN_HEIGHT);
            float len = 60.0f + (float)(i % 5) * 35.0f;
            Color streakCol = (i % 2 == 0) ? (Color){ 0, 240, 255, 180 } : (Color){ 180, 220, 255, 220 };
            DrawLineEx((Vector2){ rx, ry - len }, (Vector2){ rx, ry }, 2.0f, streakCol);
        }
    }

    float shipY = SCREEN_HEIGHT - 130.0f;

    if (t < 0.7f)
    {
        float progress = t / 0.7f;
        gunship.flameBoost = 15.0f * progress;
        gunship.position.y = shipY + sinf(t * 30.0f) * 2.0f;
    }
    else if (t < 1.9f)
    {
        float progress = (t - 0.7f) / 1.2f;
        gunship.flameBoost = 45.0f;
        gunship.position.y = shipY - (progress * progress * 520.0f);
    }
    else
    {
        float progress = (t - 1.9f) / 0.5f;
        gunship.flameBoost = 15.0f * (1.0f - progress);
        gunship.position.y = (SCREEN_HEIGHT - 90.0f) + (1.0f - progress) * 25.0f;
    }

    DrawGunship();
    gunship.position.y = SCREEN_HEIGHT - 90.0f;

    if (t < 0.7f)
    {
        int boxW = 460;
        int boxH = 140;
        DrawRectangle(SCREEN_WIDTH / 2 - boxW / 2, 220, boxW, boxH, (Color){ 10, 14, 25, 230 });
        DrawRectangleLinesEx((Rectangle){ (float)(SCREEN_WIDTH / 2 - boxW / 2), 220.0f, (float)boxW, (float)boxH }, 2.0f, (Color){ 0, 240, 255, 220 });

        DrawText("SYSTEM INITIALIZATION", SCREEN_WIDTH / 2 - 120, 238, 18, (Color){ 0, 240, 255, 255 });

        char pInfo[64];
        snprintf(pInfo, sizeof(pInfo), "PILOT CALLSIGN: [%s] LINKED", pilotName);
        DrawText(pInfo, SCREEN_WIDTH / 2 - 140, 270, 15, (Color){ 50, 255, 140, 255 });

        DrawText("NEURAL INTERFACE: SYNCHRONIZED 100%", SCREEN_WIDTH / 2 - 165, 296, 14, (Color){ 180, 220, 255, 220 });
        DrawText("SONIC DISRUPTORS: 3 CHARGES ARMED", SCREEN_WIDTH / 2 - 150, 318, 14, (Color){ 255, 215, 0, 255 });
    }
    else if (t < 1.9f)
    {
        const char* warpText = "⚡ HYPERSPACE WARP ENGAGED ⚡";
        int wW = MeasureText(warpText, 28);
        DrawText(warpText, SCREEN_WIDTH / 2 - wW / 2, 240, 28, (Color){ 0, 240, 255, 255 });

        const char* destText = "COURSE: SECTOR 01 // INTERCEPTING HOSTILE FLEET";
        int dW = MeasureText(destText, 16);
        DrawText(destText, SCREEN_WIDTH / 2 - dW / 2, 285, 16, WHITE);

        char speedStr[40];
        snprintf(speedStr, sizeof(speedStr), "VELOCITY: WARP %.1f C", 1.0f + ((t - 0.7f) / 1.2f) * 8.9f);
        int sW = MeasureText(speedStr, 18);
        DrawText(speedStr, SCREEN_WIDTH / 2 - sW / 2, 315, 18, (Color){ 255, 215, 0, 255 });
    }
    else
    {
        float flashAlpha = 1.0f - ((t - 1.9f) / 0.5f);
        if (flashAlpha > 0.0f)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade((Color){ 0, 240, 255, 255 }, flashAlpha * 0.45f));
        }

        const char* arrivalText = "WARP ARRIVAL: SECTOR 01";
        int aW = MeasureText(arrivalText, 32);
        DrawText(arrivalText, SCREEN_WIDTH / 2 - aW / 2, 260, 32, (Color){ 255, 60, 60, 255 });

        const char* engageText = "ARMADA ENGAGED // ALL WEAPONS FREE!";
        int eW = MeasureText(engageText, 18);
        DrawText(engageText, SCREEN_WIDTH / 2 - eW / 2, 305, 18, WHITE);
    }

    const char* skipHint = "[SPACE / ENTER / CLICK TO SKIP]";
    int shW = MeasureText(skipHint, 13);
    DrawText(skipHint, SCREEN_WIDTH / 2 - shW / 2, SCREEN_HEIGHT - 35, 13, (Color){ 140, 180, 220, 180 });
}

static void DrawPlayScreen(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    Vector2 shake = GetScreenShakeOffset();
    Camera2D camera = { 0 };
    camera.offset = shake;
    camera.zoom = 1.0f;

    BeginMode2D(camera);

    for (int y = 170; y < SCREEN_HEIGHT; y += 80)
    {
        DrawLine(0, y, SCREEN_WIDTH, y, (Color){ 20, 25, 40, 160 });
    }
    for (int x = 100; x < SCREEN_WIDTH; x += 100)
    {
        DrawLine(x, 120, x, SCREEN_HEIGHT, (Color){ 20, 25, 40, 160 });
    }

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

    char pilotBadge[32];
    snprintf(pilotBadge, sizeof(pilotBadge), "PILOT: %s", pilotName);
    DrawText(pilotBadge, 24, 24, 15, (Color){ 50, 255, 140, 255 });

    char lvlBadge[32];
    snprintf(lvlBadge, sizeof(lvlBadge), "SECTOR %02d/10", currentLevel);
    DrawText(lvlBadge, 24, 46, 14, (Color){ 0, 220, 255, 255 });

    int cleared = wordsClearedThisLevel;
    if (cleared > 10) cleared = 10;
    for (int p = 0; p < 10; p++)
    {
        Color pipColor = (p < cleared) ? (Color){ 0, 240, 255, 255 } : (Color){ 30, 45, 70, 255 };
        DrawRectangle(24 + p * 16, 72, 12, 8, pipColor);
        DrawRectangleLines(24 + p * 16, 72, 12, 8, (Color){ 50, 80, 120, 200 });
    }

    Vector2 mouse = GetMousePosition();
    bool sonicHover = CheckCollisionPointRec(mouse, BTN_HUD_SONIC);

    Color sonicBorderCol = (sonicWavesRemaining > 0) ?
        (sonicHover ? (Color){ 255, 240, 100, 255 } : (Color){ 0, 220, 255, 220 }) :
        (Color){ 70, 40, 40, 200 };

    Color sonicBgCol = (sonicWavesRemaining > 0) ?
        (sonicHover ? (Color){ 25, 55, 90, 255 } : (Color){ 16, 26, 46, 255 }) :
        (Color){ 22, 16, 18, 255 };

    DrawRectangleRec(BTN_HUD_SONIC, sonicBgCol);
    DrawRectangleLinesEx(BTN_HUD_SONIC, 1.5f, sonicBorderCol);

    DrawText("⚡ SONIC WAVE", (int)BTN_HUD_SONIC.x + 12, (int)BTN_HUD_SONIC.y + 10, 14, (sonicWavesRemaining > 0) ? (Color){ 255, 220, 70, 255 } : DARKGRAY);
    DrawText("[KEY: SPACE]", (int)BTN_HUD_SONIC.x + 12, (int)BTN_HUD_SONIC.y + 28, 11, (Color){ 140, 180, 220, 220 });

    for (int c = 0; c < MAX_SONIC_WAVES; c++)
    {
        int cellX = (int)BTN_HUD_SONIC.x + 12 + c * 50;
        int cellY = (int)BTN_HUD_SONIC.y + 46;

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

    DrawText("COMBAT SCORE", 423, 22, 13, (Color){ 140, 170, 210, 255 });

    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "%d", currentScore);
    int sW = MeasureText(scoreText, 24);
    DrawText(scoreText, 480 - sW / 2, 40, 24, (Color){ 0, 240, 255, 255 });

    if (currentCombo > 1)
    {
        Color comboCol = (currentCombo >= 5) ? (Color){ 255, 60, 80, 255 } :
                         (currentCombo >= 3) ? (Color){ 255, 215, 0, 255 } :
                         (Color){ 50, 255, 140, 255 };

        char comboText[24];
        snprintf(comboText, sizeof(comboText), "COMBO x%d!", currentCombo);
        int cW = MeasureText(comboText, 14);
        DrawText(comboText, 480 - cW / 2, 68, 14, comboCol);
    }
    else
    {
        DrawText("COMBO READY", 432, 68, 12, (Color){ 70, 95, 130, 255 });
    }

    DrawRectangle(568, 14, 125, 82, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 568, 14, 125, 82 }, 1.5f, (Color){ 40, 80, 140, 255 });

    DrawText("LIVE WPM", 596, 22, 13, (Color){ 140, 170, 210, 255 });

    int liveWpm = GetLiveWPM();
    char wpmText[32];
    snprintf(wpmText, sizeof(wpmText), "%d", liveWpm);
    int wpmWidth = MeasureText(wpmText, 26);

    Color liveWpmCol = (liveWpm >= 80) ? (Color){ 0, 240, 255, 255 } :
                       (liveWpm >= 50) ? (Color){ 50, 255, 140, 255 } :
                       (Color){ 200, 230, 255, 255 };

    DrawText(wpmText, 630 - wpmWidth / 2, 42, 26, liveWpmCol);

    const char* wpmSub = (liveWpm >= 80) ? "SPEED: FAST" :
                         (liveWpm >= 50) ? "SPEED: GOOD" : "SPEED: STEADY";
    int wsW = MeasureText(wpmSub, 10);
    DrawText(wpmSub, 630 - wsW / 2, 70, 10, (Color){ 120, 150, 190, 220 });

    DrawRectangle(704, 14, 82, 38, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 704, 14, 82, 38 }, 1.5f, (Color){ 40, 80, 140, 255 });

    char accuracyText[32];
    snprintf(accuracyText, sizeof(accuracyText), "%.0f%%", GetAccuracy());
    int accuracyWidth = MeasureText(accuracyText, 17);
    DrawText(accuracyText, 745 - accuracyWidth / 2, 24, 17, (Color){ 255, 220, 80, 255 });

    bool pauseHover = CheckCollisionPointRec(mouse, BTN_HUD_PAUSE);
    DrawRectangleRec(BTN_HUD_PAUSE, pauseHover ? (Color){ 0, 190, 240, 255 } : (Color){ 20, 30, 50, 220 });
    DrawRectangleLinesEx(BTN_HUD_PAUSE, 1.0f, pauseHover ? WHITE : (Color){ 0, 200, 255, 180 });
    DrawText("⏸", (int)BTN_HUD_PAUSE.x + 11, (int)BTN_HUD_PAUSE.y + 8, 16, pauseHover ? BLACK : WHITE);

    bool muteHover = CheckCollisionPointRec(mouse, BTN_HUD_MUTE);
    DrawRectangleRec(BTN_HUD_MUTE, muteHover ? (Color){ 160, 80, 200, 255 } : (Color){ 20, 30, 50, 220 });
    DrawRectangleLinesEx(BTN_HUD_MUTE, 1.0f, muteHover ? WHITE : (Color){ 140, 80, 180, 180 });
    DrawText(IsMusicMuted() ? "🔇" : "🔊", (int)BTN_HUD_MUTE.x + 9, (int)BTN_HUD_MUTE.y + 8, 16, muteHover ? BLACK : WHITE);

    if (isPaused)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));

        int panelW = 440;
        int panelH = 280;
        int panelX = SCREEN_WIDTH / 2 - panelW / 2;
        int panelY = SCREEN_HEIGHT / 2 - panelH / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 12, 16, 28, 245 });
        DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 0, 220, 255, 255 });

        DrawRectangle(panelX - 4, panelY - 4, 10, 10, (Color){ 0, 240, 255, 255 });
        DrawRectangle(panelX + panelW - 6, panelY - 4, 10, 10, (Color){ 0, 240, 255, 255 });
        DrawRectangle(panelX - 4, panelY + panelH - 6, 10, 10, (Color){ 0, 240, 255, 255 });
        DrawRectangle(panelX + panelW - 6, panelY + panelH - 6, 10, 10, (Color){ 0, 240, 255, 255 });

        const char* pTitle = "MISSION PAUSED";
        int ptW = MeasureText(pTitle, 26);
        DrawText(pTitle, SCREEN_WIDTH / 2 - ptW / 2, panelY + 20, 26, (Color){ 255, 215, 0, 255 });
        DrawLine(panelX + 30, panelY + 54, panelX + panelW - 30, panelY + 54, (Color){ 40, 100, 160, 200 });

        bool resHover = CheckCollisionPointRec(mouse, BTN_PAUSE_RESUME);
        DrawRectangleRec(BTN_PAUSE_RESUME, resHover ? (Color){ 0, 190, 240, 255 } : (Color){ 18, 45, 80, 255 });
        DrawRectangleLinesEx(BTN_PAUSE_RESUME, 1.5f, resHover ? WHITE : (Color){ 0, 220, 255, 255 });
        const char* rText = "RESUME MISSION [ESC]";
        int rtW = MeasureText(rText, 15);
        DrawText(rText, SCREEN_WIDTH / 2 - rtW / 2, (int)BTN_PAUSE_RESUME.y + 13, 15, resHover ? BLACK : WHITE);

        bool rstHover = CheckCollisionPointRec(mouse, BTN_PAUSE_RESTART);
        DrawRectangleRec(BTN_PAUSE_RESTART, rstHover ? (Color){ 255, 170, 30, 255 } : (Color){ 40, 30, 20, 255 });
        DrawRectangleLinesEx(BTN_PAUSE_RESTART, 1.5f, rstHover ? WHITE : (Color){ 220, 140, 20, 200 });
        const char* rstText = "RESTART MISSION";
        int rstW = MeasureText(rstText, 15);
        DrawText(rstText, SCREEN_WIDTH / 2 - rstW / 2, (int)BTN_PAUSE_RESTART.y + 13, 15, rstHover ? BLACK : WHITE);

        bool mnuHover = CheckCollisionPointRec(mouse, BTN_PAUSE_MENU);
        DrawRectangleRec(BTN_PAUSE_MENU, mnuHover ? (Color){ 200, 40, 50, 255 } : (Color){ 45, 20, 25, 255 });
        DrawRectangleLinesEx(BTN_PAUSE_MENU, 1.5f, mnuHover ? WHITE : (Color){ 180, 50, 60, 200 });
        const char* mnuText = "RETURN TO MENU [M]";
        int mnuW = MeasureText(mnuText, 15);
        DrawText(mnuText, SCREEN_WIDTH / 2 - mnuW / 2, (int)BTN_PAUSE_MENU.y + 13, 15, WHITE);
    }
}

static void DrawLevelClearScreen(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    int panelW = 640;
    int panelH = 580;
    int panelX = SCREEN_WIDTH / 2 - panelW / 2;
    int panelY = 90;

    DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 10, 15, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 0, 230, 255, 255 });

    DrawRectangle(panelX - 4, panelY - 4, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(panelX + panelW - 8, panelY - 4, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(panelX - 4, panelY + panelH - 8, 12, 12, (Color){ 0, 240, 255, 255 });
    DrawRectangle(panelX + panelW - 8, panelY + panelH - 8, 12, 12, (Color){ 0, 240, 255, 255 });

    DrawText("SYSTEM STATUS: SECTOR SECURED", panelX + 30, panelY + 25, 14, (Color){ 0, 240, 255, 255 });

    char clearTitle[64];
    snprintf(clearTitle, sizeof(clearTitle), "★ SECTOR %02d LIBERATED! ★", currentLevel);
    int ctW = MeasureText(clearTitle, 32);
    DrawText(clearTitle, SCREEN_WIDTH / 2 - ctW / 2, panelY + 50, 32, (Color){ 255, 215, 0, 255 });

    DrawLine(panelX + 30, panelY + 95, panelX + panelW - 30, panelY + 95, (Color){ 40, 100, 160, 200 });

    float acc = GetAccuracy();
    const char* gradeStr = "S";
    Color gradeCol = (Color){ 255, 215, 0, 255 };
    const char* gradeDesc = "SUPREME MARKSMAN";

    if (acc < 75.0f)
    {
        gradeStr = "C";
        gradeCol = (Color){ 255, 140, 40, 255 };
        gradeDesc = "COMBAT SURVIVOR";
    }
    else if (acc < 88.0f)
    {
        gradeStr = "B";
        gradeCol = (Color){ 50, 255, 140, 255 };
        gradeDesc = "TACTICAL SPECIALIST";
    }
    else if (acc < 95.0f)
    {
        gradeStr = "A";
        gradeCol = (Color){ 0, 240, 255, 255 };
        gradeDesc = "VETERAN ACE PILOT";
    }

    DrawCircle(SCREEN_WIDTH / 2, panelY + 160, 40.0f, (Color){ 16, 26, 44, 255 });
    DrawCircleLines(SCREEN_WIDTH / 2, panelY + 160, 40.0f, gradeCol);
    DrawCircleLines(SCREEN_WIDTH / 2, panelY + 160, 43.0f, Fade(gradeCol, 0.5f));

    int gW = MeasureText(gradeStr, 40);
    DrawText(gradeStr, SCREEN_WIDTH / 2 - gW / 2, panelY + 140, 40, gradeCol);

    int gdW = MeasureText(gradeDesc, 14);
    DrawText(gradeDesc, SCREEN_WIDTH / 2 - gdW / 2, panelY + 210, 14, gradeCol);

    int cardY = panelY + 240;
    int cardW = 126;
    int cardH = 75;
    int cardSpacing = 14;
    int startCardX = panelX + 46;

    int curAvgWpm = GetAverageWPM();

    int c0X = startCardX;
    DrawRectangle(c0X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c0X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 80, 140, 255 });
    DrawText("HOSTILES", c0X + 32, cardY + 12, 12, (Color){ 140, 170, 210, 255 });
    DrawText("10 / 10", c0X + 35, cardY + 38, 18, WHITE);

    int c1X = startCardX + 1 * (cardW + cardSpacing);
    DrawRectangle(c1X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c1X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 80, 140, 255 });
    DrawText("ACCURACY", c1X + 31, cardY + 12, 12, (Color){ 140, 170, 210, 255 });
    char aStr[32];
    snprintf(aStr, sizeof(aStr), "%.1f%%", acc);
    int asW = MeasureText(aStr, 20);
    DrawText(aStr, c1X + cardW / 2 - asW / 2, cardY + 36, 20, (Color){ 255, 220, 80, 255 });

    int c2X = startCardX + 2 * (cardW + cardSpacing);
    DrawRectangle(c2X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c2X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 80, 140, 255 });
    DrawText("AVG WPM", c2X + 35, cardY + 12, 12, (Color){ 140, 170, 210, 255 });
    char wStr[32];
    snprintf(wStr, sizeof(wStr), "%d WPM", curAvgWpm);
    int wsW = MeasureText(wStr, 18);
    DrawText(wStr, c2X + cardW / 2 - wsW / 2, cardY + 38, 18, (Color){ 80, 255, 150, 255 });

    int c3X = startCardX + 3 * (cardW + cardSpacing);
    DrawRectangle(c3X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c3X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 80, 140, 255 });
    DrawText("TOTAL SCORE", c3X + 25, cardY + 12, 12, (Color){ 140, 170, 210, 255 });
    char scStr[32];
    snprintf(scStr, sizeof(scStr), "%d", currentScore);
    int scW = MeasureText(scStr, 20);
    DrawText(scStr, c3X + cardW / 2 - scW / 2, cardY + 36, 20, (Color){ 0, 230, 255, 255 });

    int pPowY = cardY + 95;
    DrawRectangle(panelX + 45, pPowY, panelW - 90, 44, (Color){ 20, 40, 65, 240 });
    DrawRectangleLinesEx((Rectangle){ (float)(panelX + 45), (float)pPowY, (float)(panelW - 90), 44.0f }, 1.5f, (Color){ 0, 240, 255, 255 });

    char powStr[80];
    snprintf(powStr, sizeof(powStr), "⚡ REWARD: +1 SONIC POWER CELL RESTORED! (CHARGES: %d/3) ⚡", sonicWavesRemaining);
    int pwW = MeasureText(powStr, 14);
    DrawText(powStr, SCREEN_WIDTH / 2 - pwW / 2, pPowY + 15, 14, (Color){ 255, 220, 80, 255 });

    int nextLvl = currentLevel + 1;
    if (nextLvl > 10) nextLvl = 10;
    int nextWpm = GetTargetWPM(nextLvl);

    int intelY = pPowY + 60;
    char intelStr[96];
    if (currentLevel < 10)
    {
        snprintf(intelStr, sizeof(intelStr), "NEXT ZONE: SECTOR %02d // FLEET VELOCITY +25%%", nextLvl);
    }
    else
    {
        snprintf(intelStr, sizeof(intelStr), "FINAL VICTORY: ALL 10 SECTORS SECURED!");
    }
    int inW = MeasureText(intelStr, 14);
    DrawText(intelStr, SCREEN_WIDTH / 2 - inW / 2, intelY, 14, (Color){ 160, 200, 240, 255 });

    float progress = levelClearTimer / LEVEL_CLEAR_DURATION;
    if (progress < 0.0f) progress = 0.0f;
    int barW = panelW - 90;
    int barY = intelY + 28;
    DrawRectangle(panelX + 45, barY, barW, 8, (Color){ 20, 30, 45, 255 });
    DrawRectangle(panelX + 45, barY, (int)(barW * progress), 8, (Color){ 0, 240, 255, 255 });

    Vector2 mouse = GetMousePosition();
    bool nextHover = CheckCollisionPointRec(mouse, BTN_CLEAR_NEXT);
    DrawRectangleRec(BTN_CLEAR_NEXT, nextHover ? (Color){ 0, 200, 255, 255 } : (Color){ 16, 50, 95, 255 });
    DrawRectangleLinesEx(BTN_CLEAR_NEXT, 2.0f, nextHover ? WHITE : (Color){ 0, 230, 255, 255 });

    char btnText[64];
    snprintf(btnText, sizeof(btnText), "ENGAGE NEXT SECTOR (%.1fs) [SPACE]", levelClearTimer > 0.0f ? levelClearTimer : 0.0f);
    int bW = MeasureText(btnText, 16);
    DrawText(btnText, SCREEN_WIDTH / 2 - bW / 2, (int)BTN_CLEAR_NEXT.y + 15, 16, nextHover ? BLACK : WHITE);
}

static void DrawGameOverScreen(void)
{
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    int panelW = 680;
    int panelH = 680;
    int panelX = SCREEN_WIDTH / 2 - panelW / 2;
    int panelY = 50;

    DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 12, 16, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 220, 60, 60, 240 });

    DrawRectangle(panelX - 4, panelY - 4, 10, 10, RED);
    DrawRectangle(panelX + panelW - 6, panelY - 4, 10, 10, RED);
    DrawRectangle(panelX - 4, panelY + panelH - 6, 10, 10, RED);
    DrawRectangle(panelX + panelW - 6, panelY + panelH - 6, 10, 10, RED);

    const char* overText = (currentLevel >= 10 && wordsClearedThisLevel >= 10) ? "MISSION ACCOMPLISHED!" : "MISSION TERMINATED";
    int oW = MeasureText(overText, 34);
    DrawText(overText, SCREEN_WIDTH / 2 - oW / 2, panelY + 25, 34, (currentLevel >= 10 && wordsClearedThisLevel >= 10) ? (Color){ 255, 215, 0, 255 } : (Color){ 255, 60, 60, 255 });

    const char* debriefText = "COMBAT DEBRIEFING REPORT";
    int dW = MeasureText(debriefText, 16);
    DrawText(debriefText, SCREEN_WIDTH / 2 - dW / 2, panelY + 68, 16, (Color){ 160, 190, 220, 220 });

    DrawLine(panelX + 30, panelY + 95, panelX + panelW - 30, panelY + 95, (Color){ 80, 40, 50, 200 });

    int startStatsY = panelY + 115;
    int avgWpm = GetAverageWPM();

    char pilotReport[80];
    snprintf(pilotReport, sizeof(pilotReport), "CALLSIGN: %s // COMBAT PACE: %d AVG WPM", pilotName, avgWpm);
    DrawText(pilotReport, panelX + 50, startStatsY, 19, (Color){ 50, 255, 140, 255 });

    if (lastPlayerRank > 0 && lastPlayerRank <= 10)
    {
        char rankBanner[80];
        snprintf(rankBanner, sizeof(rankBanner), "★ NEW RECORD! RANK #%d IN HALL OF FAME! (%d AVG WPM) ★", lastPlayerRank, avgWpm);
        DrawRectangle(panelX + 40, startStatsY + 30, panelW - 80, 36, (Color){ 60, 50, 10, 220 });
        DrawRectangleLinesEx((Rectangle){ (float)(panelX + 40), (float)(startStatsY + 30), (float)(panelW - 80), 36.0f }, 1.5f, (Color){ 255, 215, 0, 255 });
        int rW = MeasureText(rankBanner, 16);
        DrawText(rankBanner, SCREEN_WIDTH / 2 - rW / 2, startStatsY + 39, 16, (Color){ 255, 215, 0, 255 });
    }
    else
    {
        char logStr[64];
        snprintf(logStr, sizeof(logStr), "RANKING: MISSION LOGGED // AVERAGE SPEED: %d WPM", avgWpm);
        DrawText(logStr, panelX + 50, startStatsY + 35, 16, (Color){ 160, 180, 210, 220 });
    }

    int cardY = startStatsY + 80;
    int cardW = 112;
    int cardH = 75;
    int cardSpacing = 10;
    int startCardX = panelX + 40;

    int c0X = startCardX;
    DrawRectangle(c0X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c0X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("SCORE", c0X + 35, cardY + 12, 12, (Color){ 120, 160, 200, 255 });
    char fScore[32];
    snprintf(fScore, sizeof(fScore), "%d", currentScore);
    int fsW = MeasureText(fScore, 20);
    DrawText(fScore, c0X + cardW / 2 - fsW / 2, cardY + 36, 20, (Color){ 0, 230, 255, 255 });

    int c1X = startCardX + 1 * (cardW + cardSpacing);
    DrawRectangle(c1X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c1X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("SECTOR", c1X + 32, cardY + 12, 12, (Color){ 120, 160, 200, 255 });
    char fLvl[16];
    snprintf(fLvl, sizeof(fLvl), "LV %d", currentLevel);
    int flW = MeasureText(fLvl, 20);
    DrawText(fLvl, c1X + cardW / 2 - flW / 2, cardY + 36, 20, WHITE);

    int c2X = startCardX + 2 * (cardW + cardSpacing);
    DrawRectangle(c2X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c2X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("ACCURACY", c2X + 26, cardY + 12, 12, (Color){ 120, 160, 200, 255 });
    char fAcc[16];
    snprintf(fAcc, sizeof(fAcc), "%.1f%%", GetAccuracy());
    int faW = MeasureText(fAcc, 20);
    DrawText(fAcc, c2X + cardW / 2 - faW / 2, cardY + 36, 20, (Color){ 255, 220, 80, 255 });

    int c3X = startCardX + 3 * (cardW + cardSpacing);
    DrawRectangle(c3X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c3X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("AVG WPM", c3X + 30, cardY + 12, 12, (Color){ 120, 160, 200, 255 });
    char fWpm[16];
    snprintf(fWpm, sizeof(fWpm), "%d", avgWpm);
    int fwW = MeasureText(fWpm, 20);
    DrawText(fWpm, c3X + cardW / 2 - fwW / 2, cardY + 36, 20, (Color){ 80, 255, 150, 255 });

    int c4X = startCardX + 4 * (cardW + cardSpacing);
    DrawRectangle(c4X, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)c4X, (float)cardY, (float)cardW, (float)cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("MAX COMBO", c4X + 22, cardY + 12, 12, (Color){ 120, 160, 200, 255 });
    char fCombo[16];
    snprintf(fCombo, sizeof(fCombo), "x%d", maxCombo);
    int fcW = MeasureText(fCombo, 20);
    DrawText(fCombo, c4X + cardW / 2 - fcW / 2, cardY + 36, 20, (Color){ 255, 180, 50, 255 });

    int miniY = cardY + 95;
    DrawLeaderboardMini(panelX + 40, miniY, panelW - 80, lastPlayerRank);

    Vector2 mouse = GetMousePosition();

    bool playHover = CheckCollisionPointRec(mouse, BTN_OVER_PLAY);
    DrawRectangleRec(BTN_OVER_PLAY, playHover ? (Color){ 0, 200, 255, 255 } : (Color){ 20, 50, 90, 255 });
    DrawRectangleLinesEx(BTN_OVER_PLAY, 1.5f, playHover ? WHITE : (Color){ 0, 220, 255, 255 });
    const char* pText = "PLAY AGAIN [ENTER]";
    int ptW = MeasureText(pText, 14);
    DrawText(pText, (int)BTN_OVER_PLAY.x + (int)BTN_OVER_PLAY.width / 2 - ptW / 2, (int)BTN_OVER_PLAY.y + 16, 14, playHover ? BLACK : WHITE);

    bool boardHover = CheckCollisionPointRec(mouse, BTN_OVER_BOARD);
    DrawRectangleRec(BTN_OVER_BOARD, boardHover ? (Color){ 0, 180, 220, 255 } : (Color){ 18, 30, 55, 255 });
    DrawRectangleLinesEx(BTN_OVER_BOARD, 1.5f, boardHover ? (Color){ 0, 240, 255, 255 } : (Color){ 40, 100, 160, 200 });
    const char* bText = "FULL LEADERBOARD [TAB]";
    int btW = MeasureText(bText, 14);
    DrawText(bText, (int)BTN_OVER_BOARD.x + (int)BTN_OVER_BOARD.width / 2 - btW / 2, (int)BTN_OVER_BOARD.y + 16, 14, boardHover ? BLACK : WHITE);

    bool menuHover = CheckCollisionPointRec(mouse, BTN_OVER_MENU);
    DrawRectangleRec(BTN_OVER_MENU, menuHover ? (Color){ 180, 40, 50, 255 } : (Color){ 40, 20, 28, 255 });
    DrawRectangleLinesEx(BTN_OVER_MENU, 1.5f, menuHover ? WHITE : (Color){ 180, 50, 60, 200 });
    const char* mText = "MAIN MENU [ESC]";
    int mtW = MeasureText(mText, 14);
    DrawText(mText, (int)BTN_OVER_MENU.x + (int)BTN_OVER_MENU.width / 2 - mtW / 2, (int)BTN_OVER_MENU.y + 16, 14, WHITE);
}

void DrawGame(void)
{
    switch (gameState)
    {
        case STATE_MENU:
            DrawStartingPage();
            break;

        case STATE_LAUNCH_ANIMATION:
            DrawLaunchAnimationScreen();
            break;

        case STATE_PLAYING:
            DrawPlayScreen();
            break;

        case STATE_LEVEL_CLEAR:
            DrawLevelClearScreen();
            break;

        case STATE_GAMEOVER:
            DrawGameOverScreen();
            break;

        case STATE_LEADERBOARD:
            DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawLeaderboardScreen(lastPlayerRank);
            break;
    }
}

void UnloadGame(void)
{
    UnloadPlayer();
    UnloadMedia();
}
