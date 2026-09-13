#include "media.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_STARS 160

typedef struct {
    float x;
    float y;
    float speed;
    float size;
    Color color;
    float twinklePhase;
} CosmicStar;

static Texture2D customBgTexture = { 0 };
static bool hasCustomBg = false;

static Music bgmMusic = { 0 };
static bool hasCustomMusic = false;
static bool isMusicMuted = false;

static CosmicStar stars[MAX_STARS];
static bool starsInitialized = false;

void InitMedia(void)
{
    const char* bgCandidates[] = {
        "assets/background.png",
        "assets/background.jpg",
        "assets/bg.png",
        "assets/bg.jpg",
        "assets/wallpaper.png"
    };

    hasCustomBg = false;
    for (int i = 0; i < 5; i++)
    {
        if (FileExists(bgCandidates[i]))
        {
            customBgTexture = LoadTexture(bgCandidates[i]);
            if (customBgTexture.id > 0)
            {
                hasCustomBg = true;
                printf("[Media] Custom background loaded successfully from: %s\n", bgCandidates[i]);
                break;
            }
        }
    }

    const char* musicCandidates[] = {
        "assets/music.mp3",
        "assets/music.ogg",
        "assets/music.wav",
        "assets/bgm.mp3",
        "assets/bgm.ogg",
        "assets/soundtrack.mp3"
    };

    hasCustomMusic = false;
    isMusicMuted = false;

    InitAudioDevice();

    if (IsAudioDeviceReady())
    {
        for (int i = 0; i < 6; i++)
        {
            if (FileExists(musicCandidates[i]))
            {
                bgmMusic = LoadMusicStream(musicCandidates[i]);
                if (bgmMusic.stream.buffer != NULL)
                {
                    bgmMusic.looping = true;
                    SetMusicVolume(bgmMusic, 0.65f);
                    PlayMusicStream(bgmMusic);
                    hasCustomMusic = true;
                    printf("[Media] Custom BGM loaded successfully from: %s\n", musicCandidates[i]);
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].x = (float)GetRandomValue(0, 800);
        stars[i].y = (float)GetRandomValue(0, 800);
        stars[i].twinklePhase = ((float)GetRandomValue(0, 360)) * (3.14159f / 180.0f);

        int layer = GetRandomValue(0, 2);
        if (layer == 0)
        {
            stars[i].speed = (float)GetRandomValue(12, 25);
            stars[i].size = 1.0f;
            stars[i].color = (Color){ 120, 140, 190, 140 };
        }
        else if (layer == 1)
        {
            stars[i].speed = (float)GetRandomValue(30, 60);
            stars[i].size = 1.5f;
            stars[i].color = (Color){ 170, 210, 255, 200 };
        }
        else
        {
            stars[i].speed = (float)GetRandomValue(80, 140);
            stars[i].size = 2.2f;
            stars[i].color = (Color){ 230, 245, 255, 255 };
        }
    }
    starsInitialized = true;
}

void UpdateMedia(void)
{
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_M))
    {
        ToggleMusicMute();
    }

    if (hasCustomMusic && !isMusicMuted && IsAudioDeviceReady())
    {
        UpdateMusicStream(bgmMusic);
    }

    if (starsInitialized)
    {
        for (int i = 0; i < MAX_STARS; i++)
        {
            stars[i].y += stars[i].speed * dt;
            if (stars[i].y > 800.0f)
            {
                stars[i].y = -10.0f;
                stars[i].x = (float)GetRandomValue(0, 800);
            }
        }
    }
}

void DrawCustomBackground(int screenWidth, int screenHeight)
{
    if (hasCustomBg && customBgTexture.id > 0)
    {
        Rectangle source = { 0, 0, (float)customBgTexture.width, (float)customBgTexture.height };
        Rectangle dest = { 0, 0, (float)screenWidth, (float)screenHeight };
        DrawTexturePro(customBgTexture, source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);

        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.40f));
    }
    else
    {
        DrawCosmicStarfield(screenWidth, screenHeight, 1.0f);
    }
}

void DrawCosmicStarfield(int screenWidth, int screenHeight, float speedMultiplier)
{
    ClearBackground((Color){ 6, 6, 14, 255 });

    DrawCircleGradient((Vector2){ (float)(screenWidth / 2), 260.0f }, 320.0f, (Color){ 16, 25, 55, 110 }, (Color){ 6, 6, 14, 0 });
    DrawCircleGradient((Vector2){ (float)(screenWidth / 2 + 150), 600.0f }, 280.0f, (Color){ 30, 14, 45, 90 }, (Color){ 6, 6, 14, 0 });
    DrawCircleGradient((Vector2){ (float)(screenWidth / 2 - 180), 480.0f }, 240.0f, (Color){ 10, 35, 45, 80 }, (Color){ 6, 6, 14, 0 });

    float time = (float)GetTime();
    for (int i = 0; i < MAX_STARS; i++)
    {
        float twinkle = 0.7f + 0.3f * sinf(stars[i].twinklePhase + time * 3.5f);
        Color c = stars[i].color;
        c.a = (unsigned char)(c.a * twinkle);

        if (stars[i].size > 2.0f)
        {
            float tail = stars[i].speed * 0.04f * speedMultiplier;
            DrawLineEx(
                (Vector2){ stars[i].x, stars[i].y - tail },
                (Vector2){ stars[i].x, stars[i].y },
                stars[i].size,
                c
            );
        }
        else
        {
            DrawCircle((int)stars[i].x, (int)stars[i].y, stars[i].size, c);
        }
    }
}

bool HasCustomBackground(void)
{
    return hasCustomBg;
}

bool HasCustomMusic(void)
{
    return hasCustomMusic;
}

void ToggleMusicMute(void)
{
    isMusicMuted = !isMusicMuted;
    if (hasCustomMusic && IsAudioDeviceReady())
    {
        if (isMusicMuted)
        {
            PauseMusicStream(bgmMusic);
        }
        else
        {
            ResumeMusicStream(bgmMusic);
        }
    }
}

bool IsMusicMuted(void)
{
    return isMusicMuted;
}

void UnloadMedia(void)
{
    if (hasCustomBg && customBgTexture.id > 0)
    {
        UnloadTexture(customBgTexture);
        hasCustomBg = false;
    }

    if (hasCustomMusic && IsAudioDeviceReady())
    {
        StopMusicStream(bgmMusic);
        UnloadMusicStream(bgmMusic);
        hasCustomMusic = false;
    }

    if (IsAudioDeviceReady())
    {
        CloseAudioDevice();
    }
}
