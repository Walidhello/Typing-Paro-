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

//--------------------------------------------------
// Global Game State
//--------------------------------------------------
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

static int lastPlayerRank = -1;
static bool nameInputActive = true;
static float cursorTimer = 0.0f;
static GameState leaderboardReturnState = STATE_MENU;

//--------------------------------------------------
// Helper: Get Target WPM for Level
//--------------------------------------------------
static int GetTargetWPM(int level)
{
    return 20 + (level - 1) * 5;
}

//--------------------------------------------------
// Calculate Accuracy
//--------------------------------------------------
float GetAccuracy(void)
{
    if (totalChars == 0)
        return 100.0f;

    return ((float)correctChars / (float)totalChars) * 100.0f;
}

//--------------------------------------------------
// Level Progression & Cheats
//--------------------------------------------------
void CheckLevelProgression(void)
{
    if (wordsClearedThisLevel >= 10 && currentLevel < 10)
    {
        currentLevel++;
        wordsClearedThisLevel = 0;
        
        int targetWPM = GetTargetWPM(currentLevel);
        spawnDelay = 60.0f / (float)targetWPM * 0.8f; 
    }
}

void SkipLevel(void)
{
    if (currentLevel < 10)
    {
        currentLevel++;
        wordsClearedThisLevel = 0;
        
        int targetWPM = GetTargetWPM(currentLevel);
        spawnDelay = 60.0f / (float)targetWPM * 0.8f; 
        
        InitEnemies();
        InitShooting();
        spawnTimer = 0.0f;
        SpawnEnemy();
    }
}

//--------------------------------------------------
// Start / Restart a New Mission
//--------------------------------------------------
void StartNewGame(void)
{
    // Clean pilot name
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

    InitEnemies();
    InitShooting();
    LoadWords("assets/words/easy.txt");
    SpawnEnemy();

    gameState = STATE_PLAYING;
}

//--------------------------------------------------
// Initialize Entire Game
//--------------------------------------------------
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
}

//--------------------------------------------------
// Update Starting Page (Menu State)
//--------------------------------------------------
static void UpdateMenu(void)
{
    float dt = GetFrameTime();
    cursorTimer += dt;
    if (cursorTimer > 1.0f) cursorTimer = 0.0f;

    // Pilot name text input
    if (nameInputActive)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            int len = (int)strlen(pilotName);
            if ((key >= 32 && key <= 126) && len < 18)
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

    // Launch mission on ENTER
    if (IsKeyPressed(KEY_ENTER))
    {
        StartNewGame();
        return;
    }

    // Open Leaderboard on TAB or L
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_L))
    {
        leaderboardReturnState = STATE_MENU;
        gameState = STATE_LEADERBOARD;
        return;
    }

    // Mouse button click detection
    Vector2 mousePos = GetMousePosition();

    // Launch button rect
    Rectangle launchBtn = { SCREEN_WIDTH / 2 - 140, 410, 280, 48 };
    if (CheckCollisionPointRec(mousePos, launchBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        StartNewGame();
        return;
    }

    // Leaderboard button rect
    Rectangle boardBtn = { SCREEN_WIDTH / 2 - 140, 470, 280, 44 };
    if (CheckCollisionPointRec(mousePos, boardBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        leaderboardReturnState = STATE_MENU;
        gameState = STATE_LEADERBOARD;
        return;
    }

    // Input box click
    Rectangle inputRect = { SCREEN_WIDTH / 2 - 160, 325, 320, 46 };
    if (CheckCollisionPointRec(mousePos, inputRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        nameInputActive = true;
    }
}

//--------------------------------------------------
// Update Active Gameplay
//--------------------------------------------------
static void UpdatePlaying(void)
{
    // Toggle Pause on ESCAPE
    if (IsKeyPressed(KEY_ESCAPE))
    {
        isPaused = !isPaused;
    }

    if (isPaused) return;

    spawnTimer += GetFrameTime();

    if(spawnTimer >= spawnDelay)
    {
        SpawnEnemy();
        spawnTimer = 0.0f;
    }

    UpdateEnemies();
    ProcessTyping();
    UpdateGunship();
    UpdateShooting();

    // Check collision with player
    for(int i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].active)
        {
            if(enemies[i].y >= player.position.y - (player.height / 2))
            {
                // Game Over triggered! Record score to leaderboard!
                int wpm = GetTargetWPM(currentLevel);
                lastPlayerRank = AddLeaderboardEntry(pilotName, currentScore, currentLevel, GetAccuracy(), wpm);
                gameState = STATE_GAMEOVER;
                return;
            }
        }
    }
}

//--------------------------------------------------
// Update Game Over State
//--------------------------------------------------
static void UpdateGameOver(void)
{
    // Play again on ENTER
    if (IsKeyPressed(KEY_ENTER))
    {
        StartNewGame();
        return;
    }

    // View full leaderboard on TAB
    if (IsKeyPressed(KEY_TAB))
    {
        leaderboardReturnState = STATE_GAMEOVER;
        gameState = STATE_LEADERBOARD;
        return;
    }

    // Return to menu on ESC
    if (IsKeyPressed(KEY_ESCAPE))
    {
        gameState = STATE_MENU;
        return;
    }

    // Mouse buttons
    Vector2 mousePos = GetMousePosition();

    // Play Again button
    Rectangle playBtn = { SCREEN_WIDTH / 2 - 240, 680, 150, 42 };
    if (CheckCollisionPointRec(mousePos, playBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        StartNewGame();
        return;
    }

    // Full Leaderboard button
    Rectangle boardBtn = { SCREEN_WIDTH / 2 - 75, 680, 160, 42 };
    if (CheckCollisionPointRec(mousePos, boardBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        leaderboardReturnState = STATE_GAMEOVER;
        gameState = STATE_LEADERBOARD;
        return;
    }

    // Menu button
    Rectangle menuBtn = { SCREEN_WIDTH / 2 + 100, 680, 140, 42 };
    if (CheckCollisionPointRec(mousePos, menuBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        gameState = STATE_MENU;
        return;
    }
}

//--------------------------------------------------
// Update Leaderboard State
//--------------------------------------------------
static void UpdateLeaderboardScreen(void)
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
    {
        gameState = leaderboardReturnState;
    }

    // Mouse click on return area
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        gameState = leaderboardReturnState;
    }
}

//--------------------------------------------------
// Master Update Function
//--------------------------------------------------
void UpdateGame(void)
{
    // Update music stream and background starfield
    UpdateMedia();

    switch (gameState)
    {
        case STATE_MENU:
            UpdateMenu();
            break;

        case STATE_PLAYING:
            UpdatePlaying();
            break;

        case STATE_GAMEOVER:
            UpdateGameOver();
            break;

        case STATE_LEADERBOARD:
            UpdateLeaderboardScreen();
            break;
    }
}

//--------------------------------------------------
// Draw Starting Page (ZType Minimalist & Dazzling Style)
//--------------------------------------------------
static void DrawStartingPage(void)
{
    // Custom Background or Procedural Starfield
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Hovering Gunship Preview at bottom
    float bobbing = sinf(GetTime() * 2.5f) * 4.0f;
    float origY = gunship.position.y;
    gunship.position.y = SCREEN_HEIGHT - 130 + bobbing;
    DrawGunship();
    gunship.position.y = origY;

    // Glowing Title: "TYPING PARO" (ZType style)
    const char* title = "TYPING PARO";
    int titleSize = 56;
    int titleW = MeasureText(title, titleSize);
    int titleX = SCREEN_WIDTH / 2 - titleW / 2;
    int titleY = 110;

    // Glow halos
    DrawText(title, titleX + 3, titleY + 3, titleSize, (Color){ 0, 80, 180, 100 });
    DrawText(title, titleX, titleY + 1, titleSize, (Color){ 0, 200, 255, 180 });
    DrawText(title, titleX, titleY, titleSize, WHITE);

    // Subtitle
    const char* sub = "TACTICAL NEURAL SPACE INTERCEPTOR";
    int subW = MeasureText(sub, 16);
    DrawText(sub, SCREEN_WIDTH / 2 - subW / 2, 175, 16, (Color){ 100, 220, 255, 220 });

    DrawLine(SCREEN_WIDTH / 2 - 200, 205, SCREEN_WIDTH / 2 + 200, 205, (Color){ 40, 120, 200, 180 });

    // Center Terminal Glass Box
    int boxW = 500;
    int boxH = 340;
    int boxX = SCREEN_WIDTH / 2 - boxW / 2;
    int boxY = 230;

    DrawRectangle(boxX, boxY, boxW, boxH, (Color){ 10, 14, 25, 235 });
    DrawRectangleLinesEx((Rectangle){ boxX, boxY, boxW, boxH }, 2.0f, (Color){ 30, 140, 230, 200 });

    // Cyber Corner Notches
    DrawRectangle(boxX - 4, boxY - 4, 10, 10, (Color){ 0, 240, 255, 255 });
    DrawRectangle(boxX + boxW - 6, boxY - 4, 10, 10, (Color){ 0, 240, 255, 255 });
    DrawRectangle(boxX - 4, boxY + boxH - 6, 10, 10, (Color){ 0, 240, 255, 255 });
    DrawRectangle(boxX + boxW - 6, boxY + boxH - 6, 10, 10, (Color){ 0, 240, 255, 255 });

    // Terminal Header
    const char* termHead = "PILOT IDENTIFICATION LINK";
    int termW = MeasureText(termHead, 17);
    DrawText(termHead, SCREEN_WIDTH / 2 - termW / 2, boxY + 20, 17, (Color){ 0, 230, 255, 255 });

    const char* prompt = "ENTER YOUR CALLSIGN TO ENGAGE:";
    int pW = MeasureText(prompt, 14);
    DrawText(prompt, SCREEN_WIDTH / 2 - pW / 2, boxY + 54, 14, (Color){ 160, 190, 220, 220 });

    // Pilot Name Input Field
    int inputW = 340;
    int inputH = 46;
    int inputX = SCREEN_WIDTH / 2 - inputW / 2;
    int inputY = boxY + 84;

    DrawRectangle(inputX, inputY, inputW, inputH, (Color){ 6, 8, 16, 255 });
    DrawRectangleLinesEx(
        (Rectangle){ inputX, inputY, inputW, inputH },
        nameInputActive ? 2.0f : 1.0f,
        nameInputActive ? (Color){ 0, 240, 255, 255 } : DARKGRAY
    );

    // Display Name with Blinking Cursor
    char displayName[32];
    if (strlen(pilotName) == 0 && !nameInputActive)
    {
        strncpy(displayName, "CALLSIGN", sizeof(displayName));
        DrawText(displayName, inputX + 20, inputY + 12, 22, (Color){ 80, 90, 110, 255 });
    }
    else
    {
        bool showCursor = (cursorTimer < 0.5f);
        if (showCursor)
        {
            snprintf(displayName, sizeof(displayName), "%s_", pilotName);
        }
        else
        {
            snprintf(displayName, sizeof(displayName), "%s ", pilotName);
        }
        int textW = MeasureText(displayName, 22);
        DrawText(displayName, SCREEN_WIDTH / 2 - textW / 2, inputY + 12, 22, (Color){ 50, 255, 140, 255 });
    }

    // Launch Button
    Vector2 mouse = GetMousePosition();
    Rectangle launchRec = { SCREEN_WIDTH / 2 - 140, boxY + 160, 280, 48 };
    bool launchHover = CheckCollisionPointRec(mouse, launchRec);

    DrawRectangleRec(launchRec, launchHover ? (Color){ 0, 180, 230, 255 } : (Color){ 16, 40, 70, 255 });
    DrawRectangleLinesEx(launchRec, 2.0f, launchHover ? WHITE : (Color){ 0, 220, 255, 255 });

    const char* launchText = "LAUNCH MISSION [ENTER]";
    int lW = MeasureText(launchText, 18);
    DrawText(launchText, SCREEN_WIDTH / 2 - lW / 2, boxY + 174, 18, launchHover ? BLACK : WHITE);

    // Leaderboard Button
    Rectangle boardRec = { SCREEN_WIDTH / 2 - 140, boxY + 224, 280, 42 };
    bool boardHover = CheckCollisionPointRec(mouse, boardRec);

    DrawRectangleRec(boardRec, boardHover ? (Color){ 30, 65, 110, 255 } : (Color){ 14, 24, 42, 255 });
    DrawRectangleLinesEx(boardRec, 1.5f, boardHover ? (Color){ 0, 240, 255, 255 } : (Color){ 40, 100, 160, 200 });

    const char* boardText = "HALL OF FAME [TAB]";
    int bW = MeasureText(boardText, 16);
    DrawText(boardText, SCREEN_WIDTH / 2 - bW / 2, boxY + 236, 16, (Color){ 180, 220, 255, 255 });

    // Bottom Help & Audio/Background Options Bar
    DrawRectangle(0, SCREEN_HEIGHT - 45, SCREEN_WIDTH, 45, (Color){ 10, 12, 20, 240 });
    DrawLine(0, SCREEN_HEIGHT - 45, SCREEN_WIDTH, SCREEN_HEIGHT - 45, (Color){ 30, 50, 80, 200 });

    // Background option indicator
    char bgStatus[64];
    if (HasCustomBackground())
        snprintf(bgStatus, sizeof(bgStatus), "BG: Custom [assets/background.png]");
    else
        snprintf(bgStatus, sizeof(bgStatus), "BG: Procedural Starfield");
    DrawText(bgStatus, 20, SCREEN_HEIGHT - 30, 14, (Color){ 120, 160, 200, 220 });

    // Music option indicator
    char musicStatus[64];
    if (HasCustomMusic())
        snprintf(musicStatus, sizeof(musicStatus), "MUSIC: %s [M to Mute]", IsMusicMuted() ? "Muted" : "Active");
    else
        snprintf(musicStatus, sizeof(musicStatus), "MUSIC: Drop assets/music.mp3 to enable");
    int mLen = MeasureText(musicStatus, 14);
    DrawText(musicStatus, SCREEN_WIDTH - mLen - 20, SCREEN_HEIGHT - 30, 14, (Color){ 120, 160, 200, 220 });
}

//--------------------------------------------------
// Draw Active Gameplay & Cyber HUD
//--------------------------------------------------
static void DrawPlayScreen(void)
{
    // Deep space custom background or starfield
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Apply Screen Shake Camera to battlefield
    Vector2 shake = GetScreenShakeOffset();
    Camera2D camera = { 0 };
    camera.offset = shake;
    camera.zoom = 1.0f;

    BeginMode2D(camera);

    // Space tactical grid lines
    for (int y = 180; y < SCREEN_HEIGHT; y += 80)
    {
        DrawLine(0, y, SCREEN_WIDTH, y, (Color){ 20, 25, 40, 160 });
    }
    for (int x = 100; x < SCREEN_WIDTH; x += 100)
    {
        DrawLine(x, 130, x, SCREEN_HEIGHT, (Color){ 20, 25, 40, 160 });
    }

    // Draw active laser bolts & trails
    DrawShootingProjectiles();

    // Draw player's gunship
    DrawGunship();

    // Draw enemy armada & target reticle
    DrawEnemies();

    // Draw shooting visual effects (muzzle flashes, explosions, sparks, shockwaves)
    DrawShootingEffects();

    EndMode2D();

    // =====================================================
    // CYBER HUD HEADER (Stationary & Dazzling)
    // =====================================================
    DrawRectangle(0, 0, SCREEN_WIDTH, 125, (Color){ 10, 13, 24, 245 });
    DrawLine(0, 125, SCREEN_WIDTH, 125, (Color){ 0, 200, 255, 200 });

    // Angled accent corner lines
    DrawLine(0, 126, 80, 126, (Color){ 0, 240, 255, 255 });
    DrawLine(SCREEN_WIDTH - 80, 126, SCREEN_WIDTH, 126, (Color){ 0, 240, 255, 255 });

    // Logo & Pilot Card (Left)
    DrawText("TYPING PARO", 22, 16, 26, (Color){ 0, 230, 255, 255 });

    char pilotBadge[40];
    snprintf(pilotBadge, sizeof(pilotBadge), "PILOT: %s", pilotName);
    DrawText(pilotBadge, 24, 48, 15, (Color){ 50, 255, 140, 255 });

    char lvlBadge[32];
    snprintf(lvlBadge, sizeof(lvlBadge), "SECTOR LEVEL %02d/10", currentLevel);
    DrawText(lvlBadge, 24, 68, 14, (Color){ 160, 190, 220, 220 });

    // Progress bar for wave pips
    int cleared = wordsClearedThisLevel;
    if (cleared > 10) cleared = 10;
    for (int p = 0; p < 10; p++)
    {
        Color pipColor = (p < cleared) ? (Color){ 0, 230, 255, 255 } : (Color){ 30, 45, 70, 255 };
        DrawRectangle(24 + p * 15, 88, 11, 6, pipColor);
    }

    // Live Score Card (Center)
    DrawRectangle(260, 16, 170, 85, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 260, 16, 170, 85 }, 1.5f, (Color){ 40, 80, 140, 255 });

    DrawText("COMBAT SCORE", 280, 26, 14, (Color){ 140, 170, 210, 255 });

    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "%d", currentScore);
    int sW = MeasureText(scoreText, 26);
    DrawText(scoreText, 345 - sW / 2, 46, 26, (Color){ 0, 240, 255, 255 });

    if (currentCombo > 1)
    {
        char comboText[24];
        snprintf(comboText, sizeof(comboText), "COMBO x%d!", currentCombo);
        int cW = MeasureText(comboText, 14);
        DrawText(comboText, 345 - cW / 2, 77, 14, (Color){ 255, 215, 0, 255 });
    }

    // Target WPM Card
    DrawRectangle(445, 16, 160, 85, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 445, 16, 160, 85 }, 1.5f, (Color){ 40, 80, 140, 255 });

    DrawText("TARGET WPM", 475, 26, 14, (Color){ 140, 170, 210, 255 });

    char wpmText[32];
    snprintf(wpmText, sizeof(wpmText), "%d", GetTargetWPM(currentLevel));
    int wpmWidth = MeasureText(wpmText, 28);
    DrawText(wpmText, 525 - wpmWidth / 2, 50, 28, (Color){ 50, 255, 140, 255 });

    // Accuracy Card
    DrawRectangle(620, 16, 160, 85, (Color){ 16, 22, 38, 255 });
    DrawRectangleLinesEx((Rectangle){ 620, 16, 160, 85 }, 1.5f, (Color){ 40, 80, 140, 255 });

    DrawText("ACCURACY", 655, 26, 14, (Color){ 140, 170, 210, 255 });

    char accuracyText[32];
    snprintf(accuracyText, sizeof(accuracyText), "%.1f%%", GetAccuracy());
    int accuracyWidth = MeasureText(accuracyText, 28);
    DrawText(accuracyText, 700 - accuracyWidth / 2, 50, 28, (Color){ 255, 220, 80, 255 });

    // Pause Screen Overlay
    if (isPaused)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));
        DrawRectangle(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 90, 400, 180, (Color){ 12, 16, 28, 240 });
        DrawRectangleLinesEx((Rectangle){ SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 90, 400, 180 }, 2.0f, (Color){ 0, 220, 255, 255 });

        DrawText("SYSTEM PAUSED", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 40, 32, (Color){ 255, 215, 0, 255 });
        DrawText("PRESS [ESC] TO RESUME MISSION", SCREEN_WIDTH / 2 - 145, SCREEN_HEIGHT / 2 + 15, 17, WHITE);
    }
}

//--------------------------------------------------
// Draw Game Over & Mission Debriefing Screen
//--------------------------------------------------
static void DrawGameOverScreen(void)
{
    // Draw background
    DrawCustomBackground(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Dark cyber debriefing panel
    int panelW = 680;
    int panelH = 680;
    int panelX = SCREEN_WIDTH / 2 - panelW / 2;
    int panelY = 50;

    DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 12, 16, 28, 245 });
    DrawRectangleLinesEx((Rectangle){ panelX, panelY, panelW, panelH }, 2.0f, (Color){ 220, 60, 60, 240 });

    // Corner accents
    DrawRectangle(panelX - 4, panelY - 4, 10, 10, RED);
    DrawRectangle(panelX + panelW - 6, panelY - 4, 10, 10, RED);
    DrawRectangle(panelX - 4, panelY + panelH - 6, 10, 10, RED);
    DrawRectangle(panelX + panelW - 6, panelY + panelH - 6, 10, 10, RED);

    // Title
    const char* overText = "MISSION TERMINATED";
    int oW = MeasureText(overText, 36);
    DrawText(overText, SCREEN_WIDTH / 2 - oW / 2, panelY + 25, 36, (Color){ 255, 60, 60, 255 });

    const char* debriefText = "COMBAT DEBRIEFING REPORT";
    int dW = MeasureText(debriefText, 16);
    DrawText(debriefText, SCREEN_WIDTH / 2 - dW / 2, panelY + 68, 16, (Color){ 160, 190, 220, 220 });

    DrawLine(panelX + 30, panelY + 95, panelX + panelW - 30, panelY + 95, (Color){ 80, 40, 50, 200 });

    // Stats Grid
    int startStatsY = panelY + 115;

    // Pilot name & Rank achievement banner
    char pilotReport[64];
    snprintf(pilotReport, sizeof(pilotReport), "CALLSIGN: %s", pilotName);
    DrawText(pilotReport, panelX + 50, startStatsY, 20, (Color){ 50, 255, 140, 255 });

    if (lastPlayerRank > 0 && lastPlayerRank <= 10)
    {
        char rankBanner[64];
        snprintf(rankBanner, sizeof(rankBanner), "★ NEW RECORD! RANK #%d IN HALL OF FAME! ★", lastPlayerRank);
        DrawRectangle(panelX + 40, startStatsY + 30, panelW - 80, 36, (Color){ 60, 50, 10, 220 });
        DrawRectangleLinesEx((Rectangle){ panelX + 40, startStatsY + 30, panelW - 80, 36 }, 1.5f, (Color){ 255, 215, 0, 255 });
        int rW = MeasureText(rankBanner, 17);
        DrawText(rankBanner, SCREEN_WIDTH / 2 - rW / 2, startStatsY + 39, 17, (Color){ 255, 215, 0, 255 });
    }
    else
    {
        DrawText("RANKING: MISSION LOGGED", panelX + 50, startStatsY + 35, 17, (Color){ 160, 180, 210, 220 });
    }

    // Stats Cards Row
    int cardY = startStatsY + 80;
    int cardW = 135;
    int cardH = 75;

    // Final Score Card
    DrawRectangle(panelX + 40, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ panelX + 40, cardY, cardW, cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("FINAL SCORE", panelX + 52, cardY + 12, 13, (Color){ 120, 160, 200, 255 });
    char fScore[32];
    snprintf(fScore, sizeof(fScore), "%d", currentScore);
    int fsW = MeasureText(fScore, 22);
    DrawText(fScore, panelX + 40 + cardW / 2 - fsW / 2, cardY + 36, 22, (Color){ 0, 230, 255, 255 });

    // Sector Level Card
    DrawRectangle(panelX + 190, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ panelX + 190, cardY, cardW, cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("SECTOR", panelX + 225, cardY + 12, 13, (Color){ 120, 160, 200, 255 });
    char fLvl[16];
    snprintf(fLvl, sizeof(fLvl), "LV %d", currentLevel);
    int flW = MeasureText(fLvl, 22);
    DrawText(fLvl, panelX + 190 + cardW / 2 - flW / 2, cardY + 36, 22, WHITE);

    // Accuracy Card
    DrawRectangle(panelX + 340, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ panelX + 340, cardY, cardW, cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("ACCURACY", panelX + 365, cardY + 12, 13, (Color){ 120, 160, 200, 255 });
    char fAcc[16];
    snprintf(fAcc, sizeof(fAcc), "%.1f%%", GetAccuracy());
    int faW = MeasureText(fAcc, 22);
    DrawText(fAcc, panelX + 340 + cardW / 2 - faW / 2, cardY + 36, 22, (Color){ 255, 220, 80, 255 });

    // Max Combo Card
    DrawRectangle(panelX + 490, cardY, cardW, cardH, (Color){ 18, 24, 40, 255 });
    DrawRectangleLinesEx((Rectangle){ panelX + 490, cardY, cardW, cardH }, 1.5f, (Color){ 40, 90, 160, 255 });
    DrawText("MAX COMBO", panelX + 515, cardY + 12, 13, (Color){ 120, 160, 200, 255 });
    char fCombo[16];
    snprintf(fCombo, sizeof(fCombo), "x%d", maxCombo);
    int fcW = MeasureText(fCombo, 22);
    DrawText(fCombo, panelX + 490 + cardW / 2 - fcW / 2, cardY + 36, 22, (Color){ 50, 255, 140, 255 });

    // Mini Leaderboard Section
    int miniY = cardY + 95;
    DrawLeaderboardMini(panelX + 40, miniY, panelW - 80, lastPlayerRank);

    // Buttons
    Vector2 mouse = GetMousePosition();

    // 1. Play Again
    Rectangle playBtn = { panelX + 40, panelY + panelH - 75, 180, 46 };
    bool playHover = CheckCollisionPointRec(mouse, playBtn);
    DrawRectangleRec(playBtn, playHover ? (Color){ 0, 200, 255, 255 } : (Color){ 20, 50, 90, 255 });
    DrawRectangleLinesEx(playBtn, 1.5f, playHover ? WHITE : (Color){ 0, 220, 255, 255 });
    const char* pText = "PLAY AGAIN [ENTER]";
    int ptW = MeasureText(pText, 14);
    DrawText(pText, panelX + 40 + 90 - ptW / 2, panelY + panelH - 60, 14, playHover ? BLACK : WHITE);

    // 2. Full Leaderboard
    Rectangle boardBtn = { panelX + 240, panelY + panelH - 75, 200, 46 };
    bool boardHover = CheckCollisionPointRec(mouse, boardBtn);
    DrawRectangleRec(boardBtn, boardHover ? (Color){ 0, 180, 220, 255 } : (Color){ 18, 30, 55, 255 });
    DrawRectangleLinesEx(boardBtn, 1.5f, boardHover ? (Color){ 0, 240, 255, 255 } : (Color){ 40, 100, 160, 200 });
    const char* bText = "FULL LEADERBOARD [TAB]";
    int btW = MeasureText(bText, 14);
    DrawText(bText, panelX + 240 + 100 - btW / 2, panelY + panelH - 60, 14, boardHover ? BLACK : WHITE);

    // 3. Menu
    Rectangle menuBtn = { panelX + 460, panelY + panelH - 75, 180, 46 };
    bool menuHover = CheckCollisionPointRec(mouse, menuBtn);
    DrawRectangleRec(menuBtn, menuHover ? (Color){ 160, 40, 50, 255 } : (Color){ 40, 20, 28, 255 });
    DrawRectangleLinesEx(menuBtn, 1.5f, menuHover ? WHITE : (Color){ 180, 50, 60, 200 });
    const char* mText = "MAIN MENU [ESC]";
    int mtW = MeasureText(mText, 14);
    DrawText(mText, panelX + 460 + 90 - mtW / 2, panelY + panelH - 60, 14, WHITE);
}

//--------------------------------------------------
// Master Draw Function
//--------------------------------------------------
void DrawGame(void)
{
    switch (gameState)
    {
        case STATE_MENU:
            DrawStartingPage();
            break;

        case STATE_PLAYING:
            DrawPlayScreen();
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

//--------------------------------------------------
// Unload Game
//--------------------------------------------------
void UnloadGame(void)
{
    UnloadPlayer();
    UnloadMedia();
}
