#include "media.h"
#include <stdio.h>
#include <math.h>

#define MAX_STARS 160
#define LASER_VOICE_COUNT 4

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
static Sound laserVoices[LASER_VOICE_COUNT] = { 0 };
static int nextLaserVoice = 0;
static bool hasLaserSound = false;
static Sound misfireSound = { 0 };
static bool hasMisfireSound = false;
static CosmicStar stars[MAX_STARS];

void InitMedia(void)
{
    const char* bgFiles[] = { "assets/background.png", "assets/background.jpg", "assets/bg.png", "assets/bg.jpg", "assets/wallpaper.png" };
    hasCustomBg = false;
    for (int i = 0; i < 5; i++)
    {
        if (FileExists(bgFiles[i]))
        {
            customBgTexture = LoadTexture(bgFiles[i]);
            if (customBgTexture.id > 0) { hasCustomBg = true; break; }
        }
    }

    isMusicMuted = false;
    hasCustomMusic = false;
    InitAudioDevice();

    if (IsAudioDeviceReady())
    {
        const char* musFiles[] = { "assets/music.mp3", "assets/music.ogg", "assets/music.wav", "assets/bgm.mp3", "assets/bgm.ogg", "assets/soundtrack.mp3" };
        for (int i = 0; i < 6; i++)
        {
            if (FileExists(musFiles[i]))
            {
                bgmMusic = LoadMusicStream(musFiles[i]);
                if (bgmMusic.stream.buffer != NULL)
                {
                    bgmMusic.looping = true;
                    SetMusicVolume(bgmMusic, 0.65f);
                    PlayMusicStream(bgmMusic);
                    hasCustomMusic = true;
                    break;
                }
            }
        }

        if (FileExists("assets/laser.wav"))
        {
            laserVoices[0] = LoadSound("assets/laser.wav");
            if (IsSoundValid(laserVoices[0]))
            {
                SetSoundVolume(laserVoices[0], 0.75f);
                for (int i = 1; i < LASER_VOICE_COUNT; i++)
                {
                    laserVoices[i] = LoadSoundAlias(laserVoices[0]);
                    SetSoundVolume(laserVoices[i], 0.75f);
                }
                hasLaserSound = true;
            }
        }

        if (FileExists("assets/misfire.wav"))
        {
            misfireSound = LoadSound("assets/misfire.wav");
            if (IsSoundValid(misfireSound))
            {
                SetSoundVolume(misfireSound, 0.65f);
                hasMisfireSound = true;
            }
        }
    }

    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].x = (float)GetRandomValue(0, 800);
        stars[i].y = (float)GetRandomValue(0, 800);
        stars[i].twinklePhase = ((float)GetRandomValue(0, 360)) * (3.14159f / 180.0f);
        int layer = GetRandomValue(0, 2);
        stars[i].speed = (float)(layer == 0 ? GetRandomValue(12, 25) : (layer == 1 ? GetRandomValue(30, 60) : GetRandomValue(80, 140)));
        stars[i].size = (layer == 0 ? 1.0f : (layer == 1 ? 1.5f : 2.2f));
        stars[i].color = (layer == 0 ? (Color){ 120, 140, 190, 140 } : (layer == 1 ? (Color){ 170, 210, 255, 200 } : (Color){ 230, 245, 255, 255 }));
    }
}

void UpdateMedia(void)
{
    float dt = GetFrameTime();
    if (hasCustomMusic && !isMusicMuted && IsAudioDeviceReady())
        UpdateMusicStream(bgmMusic);

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

void DrawCustomBackground(int screenWidth, int screenHeight)
{
    if (hasCustomBg && customBgTexture.id > 0)
    {
        DrawTexturePro(customBgTexture,
                       (Rectangle){ 0, 0, (float)customBgTexture.width, (float)customBgTexture.height },
                       (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight },
                       (Vector2){ 0, 0 }, 0.0f, WHITE);
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
    DrawCircleGradient((Vector2){ screenWidth / 2.0f, 260.0f }, 320.0f, (Color){ 16, 25, 55, 110 }, (Color){ 6, 6, 14, 0 });
    DrawCircleGradient((Vector2){ screenWidth / 2.0f + 150.0f, 600.0f }, 280.0f, (Color){ 30, 14, 45, 90 }, (Color){ 6, 6, 14, 0 });
    DrawCircleGradient((Vector2){ screenWidth / 2.0f - 180.0f, 480.0f }, 240.0f, (Color){ 10, 35, 45, 80 }, (Color){ 6, 6, 14, 0 });

    float time = (float)GetTime();
    for (int i = 0; i < MAX_STARS; i++)
    {
        float twinkle = 0.7f + 0.3f * sinf(stars[i].twinklePhase + time * 3.5f);
        Color c = stars[i].color;
        c.a = (unsigned char)(c.a * twinkle);

        if (stars[i].size > 2.0f)
            DrawLineEx((Vector2){ stars[i].x, stars[i].y - stars[i].speed * 0.04f * speedMultiplier }, (Vector2){ stars[i].x, stars[i].y }, stars[i].size, c);
        else
            DrawCircle((int)stars[i].x, (int)stars[i].y, stars[i].size, c);
    }
}

bool HasCustomBackground(void) { return hasCustomBg; }
bool HasCustomMusic(void) { return hasCustomMusic; }
bool IsMusicMuted(void) { return isMusicMuted; }

void ToggleMusicMute(void)
{
    isMusicMuted = !isMusicMuted;
    if (hasCustomMusic && IsAudioDeviceReady())
    {
        if (isMusicMuted) PauseMusicStream(bgmMusic);
        else ResumeMusicStream(bgmMusic);
    }
}

void PlayLaserSound(bool isKillShot)
{
    if (hasLaserSound && IsAudioDeviceReady())
    {
        Sound snd = laserVoices[nextLaserVoice];
        nextLaserVoice = (nextLaserVoice + 1) % LASER_VOICE_COUNT;
        SetSoundPitch(snd, isKillShot ? 0.85f : (0.96f + (float)GetRandomValue(0, 8) * 0.01f));
        SetSoundVolume(snd, isKillShot ? 0.95f : 0.75f);
        PlaySound(snd);
    }
}

void PlayMisfireSound(void)
{
    if (hasMisfireSound && IsAudioDeviceReady())
        PlaySound(misfireSound);
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
    if (hasLaserSound && IsAudioDeviceReady())
    {
        for (int i = 1; i < LASER_VOICE_COUNT; i++)
            UnloadSoundAlias(laserVoices[i]);
        UnloadSound(laserVoices[0]);
        hasLaserSound = false;
    }
    if (hasMisfireSound && IsAudioDeviceReady())
    {
        UnloadSound(misfireSound);
        hasMisfireSound = false;
    }
    if (IsAudioDeviceReady())
        CloseAudioDevice();
}
