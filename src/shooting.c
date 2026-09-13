#include "shooting.h"
#include "gunship.h"
#include "enemy.h"
#include "game.h"
#include "media.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI_FLOAT 3.14159265358979323846f

static LaserProjectile projectiles[MAX_PROJECTILES];
static VFXParticle particles[MAX_PARTICLES];
static MuzzleFlash muzzleFlashes[MAX_MUZZLE_FLASHES];
static Shockwave shockwaves[MAX_SHOCKWAVES];
static FloatText floatTexts[MAX_FLOAT_TEXTS];

static float screenShake = 0.0f;
static Vector2 shakeOffset = { 0.0f, 0.0f };
static int lastCannon = 0;

static Color LerpColor(Color c1, Color c2, float t)
{
    t = fmaxf(0.0f, fminf(1.0f, t));
    return (Color){
        (unsigned char)(c1.r + (c2.r - c1.r) * t),
        (unsigned char)(c1.g + (c2.g - c1.g) * t),
        (unsigned char)(c1.b + (c2.b - c1.b) * t),
        (unsigned char)(c1.a + (c2.a - c1.a) * t)
    };
}

void InitShooting(void)
{
    screenShake = 0.0f;
    shakeOffset = (Vector2){ 0.0f, 0.0f };
    lastCannon = 0;
    memset(projectiles, 0, sizeof(projectiles));
    memset(particles, 0, sizeof(particles));
    memset(muzzleFlashes, 0, sizeof(muzzleFlashes));
    memset(shockwaves, 0, sizeof(shockwaves));
    memset(floatTexts, 0, sizeof(floatTexts));
}

void AddMuzzleFlash(Vector2 pos, Color color)
{
    for (int i = 0; i < MAX_MUZZLE_FLASHES; i++)
    {
        if (!muzzleFlashes[i].active)
        {
            muzzleFlashes[i] = (MuzzleFlash){ true, pos, 0.09f, 0.09f, 12.0f, color };
            break;
        }
    }
}

static void SpawnParticle(Vector2 pos, Vector2 vel, float size, float lifetime,
                          Color startCol, Color endCol, float drag, bool isSmoke)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!particles[i].active)
        {
            particles[i] = (VFXParticle){ true, pos, vel, size, lifetime, lifetime, startCol, endCol, drag, isSmoke };
            break;
        }
    }
}

void AddExhaustParticle(Vector2 pos, Vector2 vel, Color color)
{
    float life = (float)GetRandomValue(15, 30) / 100.0f;
    float size = (float)GetRandomValue(2, 4);
    SpawnParticle(pos, vel, size, life, color, (Color){ 20, 60, 120, 0 }, 1.8f, false);
}

static void AddShockwave(Vector2 pos, float radius, float maxRadius, float speed, float thickness, Color color)
{
    for (int i = 0; i < MAX_SHOCKWAVES; i++)
    {
        if (!shockwaves[i].active)
        {
            shockwaves[i] = (Shockwave){ true, pos, radius, maxRadius, speed, thickness, color };
            break;
        }
    }
}

void CreateHitSparks(Vector2 pos, Color color, int count)
{
    for (int i = 0; i < count; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(90, 260);
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        float size = (float)GetRandomValue(2, 4);
        float life = (float)GetRandomValue(15, 35) / 100.0f;
        Color sparkColor = (i % 3 == 0) ? WHITE : color;
        SpawnParticle(pos, vel, size, life, sparkColor, (Color){ sparkColor.r, sparkColor.g, sparkColor.b, 0 }, 2.5f, false);
    }
    AddShockwave(pos, 4.0f, 28.0f, 220.0f, 2.0f, color);
}

void AddFloatText(Vector2 pos, const char* text, Color color, int fontSize)
{
    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        if (!floatTexts[i].active)
        {
            floatTexts[i] = (FloatText){ true, pos, "", 0.85f, 0.85f, color, fontSize };
            strncpy(floatTexts[i].text, text, sizeof(floatTexts[i].text) - 1);
            break;
        }
    }
}

void CreateExplosion(Vector2 pos, EnemyType type, const char* word)
{
    int particleCount = (type == E_HARD_BOSS) ? 90 : ((type == E_MEDIUM_BOSS) ? 65 : 40);
    float baseSpeed = (type == E_HARD_BOSS) ? 400.0f : ((type == E_MEDIUM_BOSS) ? 320.0f : 240.0f);
    float shockwaveMax = (type == E_HARD_BOSS) ? 125.0f : ((type == E_MEDIUM_BOSS) ? 95.0f : 70.0f);
    Color primaryColor = (type == E_HARD_BOSS) ? (Color){ 220, 60, 255, 255 } :
                         ((type == E_MEDIUM_BOSS) ? (Color){ 255, 180, 20, 255 } : (Color){ 255, 120, 30, 255 });
    Color secondaryColor = (type == E_HARD_BOSS) ? (Color){ 80, 220, 255, 255 } :
                           ((type == E_MEDIUM_BOSS) ? (Color){ 255, 240, 120, 255 } : (Color){ 255, 220, 50, 255 });
    screenShake = fmaxf(screenShake, (type == E_HARD_BOSS) ? 11.0f : ((type == E_MEDIUM_BOSS) ? 7.5f : 4.5f));

    for (int i = 0; i < particleCount / 3; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(40, (int)(baseSpeed * 0.6f));
        SpawnParticle(pos, (Vector2){ cosf(angle) * speed, sinf(angle) * speed },
                      (float)GetRandomValue(6, 14), (float)GetRandomValue(35, 65) / 100.0f,
                      secondaryColor, (Color){ 40, 35, 45, 0 }, 2.5f, true);
    }

    for (int i = 0; i < particleCount; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(100, (int)baseSpeed);
        Color startC = (i % 2 == 0) ? WHITE : primaryColor;
        SpawnParticle(pos, (Vector2){ cosf(angle) * speed, sinf(angle) * speed },
                      (float)GetRandomValue(2, 5), (float)GetRandomValue(25, 55) / 100.0f,
                      startC, Fade(secondaryColor, 0.0f), 3.2f, false);
    }

    AddShockwave(pos, 8.0f, shockwaveMax, 280.0f, 3.5f, secondaryColor);
    if (type != E_NORMAL)
        AddShockwave(pos, 4.0f, shockwaveMax * 1.3f, 360.0f, 2.0f, primaryColor);

    if (word && word[0])
    {
        char buffer[40];
        snprintf(buffer, sizeof(buffer), "+%s", word);
        AddFloatText((Vector2){ pos.x, pos.y - 25.0f }, buffer,
                     (type == E_NORMAL) ? (Color){ 255, 230, 80, 255 } : (Color){ 255, 100, 220, 255 }, 20);
    }
}

static void SpawnProjectile(Vector2 origin, Vector2 target,
                            bool isKill, EnemyType enemyType, const char* word,
                            Color coreCol, Color glowCol, float speed, float length, float width)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!projectiles[i].active)
        {
            LaserProjectile* p = &projectiles[i];
            p->active = true;
            p->position = origin;
            p->target = target;
            p->isKillShot = isKill;
            p->enemyType = enemyType;
            if (word) strncpy(p->targetWord, word, sizeof(p->targetWord) - 1);
            else p->targetWord[0] = '\0';
            p->coreColor = coreCol;
            p->glowColor = glowCol;
            p->speed = speed;
            p->length = length;
            p->width = width;

            float dx = target.x - origin.x, dy = target.y - origin.y;
            float dist = sqrtf(dx * dx + dy * dy);
            p->velocity = (dist > 0.001f) ? (Vector2){ (dx / dist) * speed, (dy / dist) * speed } : (Vector2){ 0.0f, -speed };

            for (int t = 0; t < 6; t++) p->trail[t] = origin;
            p->trailCount = 1;
            break;
        }
    }
}

void FireLaserAtPosition(Vector2 targetPos, EnemyType enemyType, const char* word, bool isKillShot)
{
    PlayLaserSound(isKillShot);
    if (!isKillShot)
    {
        // Alternate between left and right cannons
        Vector2 muzzlePos = (lastCannon == 0) ? GetGunshipLeftMuzzle() : GetGunshipRightMuzzle();
        if (lastCannon == 0) { gunship.cannonGlowLeft = 0.25f; lastCannon = 1; }
        else { gunship.cannonGlowRight = 0.25f; lastCannon = 0; }

        AddMuzzleFlash(muzzlePos, (Color){ 0, 240, 255, 255 });
        gunship.recoilY = 3.2f;
        gunship.flameBoost = 12.0f;
        SpawnProjectile(muzzlePos, targetPos, false, enemyType, word,
                        (Color){ 240, 255, 255, 255 }, (Color){ 0, 220, 255, 200 }, 3000.0f, 26.0f, 4.0f);
    }
    else
    {
        Vector2 leftMuzzle = GetGunshipLeftMuzzle();
        Vector2 rightMuzzle = GetGunshipRightMuzzle();
        gunship.cannonGlowLeft = 0.45f;
        gunship.cannonGlowRight = 0.45f;

        Color glowCol = (enemyType == E_HARD_BOSS) ? (Color){ 255, 80, 240, 240 } :
                        ((enemyType == E_MEDIUM_BOSS) ? (Color){ 255, 190, 40, 240 } : (Color){ 40, 255, 200, 240 });

        AddMuzzleFlash(leftMuzzle, glowCol);
        AddMuzzleFlash(rightMuzzle, glowCol);
        gunship.recoilY = 6.0f;
        gunship.flameBoost = 22.0f;

        SpawnProjectile(leftMuzzle, targetPos, true, enemyType, word, WHITE, glowCol, 3400.0f, 38.0f, 6.0f);
        SpawnProjectile(rightMuzzle, targetPos, false, enemyType, word, WHITE, glowCol, 3400.0f, 38.0f, 6.0f);
    }
}

void TriggerMisfire(void)
{
    PlayMisfireSound();
    gunship.misfireTimer = 0.15f;
    float shipY = gunship.position.y + gunship.recoilY;
    for (int side = -1; side <= 1; side += 2)
    {
        SpawnParticle((Vector2){ gunship.position.x + side * 57.0f, shipY + 10.0f },
                      (Vector2){ (float)GetRandomValue(side * 10, side * 40), (float)GetRandomValue(-20, 20) },
                      4.0f, 0.15f, RED, Fade(DARKGRAY, 0.0f), 2.0f, true);
    }
}

void UpdateShooting(void)
{
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            LaserProjectile* p = &projectiles[i];
            if (p->trailCount < 6) p->trailCount++;
            for (int t = p->trailCount - 1; t > 0; t--)
                p->trail[t] = p->trail[t - 1];
            p->trail[0] = p->position;

            float dx = p->target.x - p->position.x;
            float dy = p->target.y - p->position.y;
            float dist = sqrtf(dx * dx + dy * dy);
            float step = p->speed * dt;

            if (dist <= step || dist < 25.0f)
            {
                p->active = false;
                if (!p->isKillShot) CreateHitSparks(p->target, p->glowColor, 9);
                else CreateExplosion(p->target, p->enemyType, p->targetWord);
            }
            else
            {
                p->velocity = (Vector2){ (dx / dist) * p->speed, (dy / dist) * p->speed };
                p->position.x += p->velocity.x * dt;
                p->position.y += p->velocity.y * dt;
                if (p->position.y < -120.0f || p->position.y > SCREEN_HEIGHT + 60.0f ||
                    p->position.x < -100.0f || p->position.x > SCREEN_WIDTH + 100.0f)
                    p->active = false;
            }
        }
    }

    for (int i = 0; i < MAX_MUZZLE_FLASHES; i++)
        if (muzzleFlashes[i].active && (muzzleFlashes[i].timer -= dt) <= 0.0f)
            muzzleFlashes[i].active = false;

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (particles[i].active)
        {
            VFXParticle* pt = &particles[i];
            if ((pt->timer -= dt) <= 0.0f)
            {
                pt->active = false;
            }
            else
            {
                pt->velocity.x *= (1.0f - pt->drag * dt);
                pt->velocity.y *= (1.0f - pt->drag * dt);
                pt->position.x += pt->velocity.x * dt;
                pt->position.y += pt->velocity.y * dt;
                if (pt->isSmoke) pt->size += 7.0f * dt;
            }
        }
    }

    for (int i = 0; i < MAX_SHOCKWAVES; i++)
        if (shockwaves[i].active && (shockwaves[i].radius += shockwaves[i].speed * dt) >= shockwaves[i].maxRadius)
            shockwaves[i].active = false;

    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        if (floatTexts[i].active)
        {
            floatTexts[i].position.y -= 32.0f * dt;
            if ((floatTexts[i].timer -= dt) <= 0.0f) floatTexts[i].active = false;
        }
    }

    if (screenShake > 0.0f)
    {
        screenShake = fmaxf(0.0f, screenShake - 24.0f * dt);
        shakeOffset.x = ((float)GetRandomValue(-100, 100) / 100.0f) * screenShake;
        shakeOffset.y = ((float)GetRandomValue(-100, 100) / 100.0f) * screenShake;
    }
    else
    {
        shakeOffset = (Vector2){ 0.0f, 0.0f };
    }
}

Vector2 GetScreenShakeOffset(void) { return shakeOffset; }

void DrawShootingProjectiles(void)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            LaserProjectile* p = &projectiles[i];
            float dx = p->velocity.x, dy = p->velocity.y;
            float len = sqrtf(dx * dx + dy * dy);
            if (len < 0.001f) continue;

            Vector2 dir = { dx / len, dy / len };
            Vector2 tip = p->position;
            Vector2 tail = { tip.x - dir.x * p->length, tip.y - dir.y * p->length };

            for (int t = 0; t < p->trailCount - 1; t++)
            {
                float trailProgress = 1.0f - ((float)t / (float)p->trailCount);
                DrawLineEx(p->trail[t], p->trail[t + 1], p->width * 0.8f * trailProgress, Fade(p->glowColor, 0.35f * trailProgress));
            }

            DrawLineEx(tail, tip, p->width * 2.6f, Fade(p->glowColor, 0.35f));
            DrawLineEx(tail, tip, p->width * 1.5f, Fade(p->glowColor, 0.85f));
            DrawLineEx((Vector2){ tip.x - dir.x * (p->length * 0.7f), tip.y - dir.y * (p->length * 0.7f) }, tip, p->width * 0.6f, p->coreColor);

            DrawCircleV(tip, p->width * 1.3f, Fade(p->glowColor, 0.9f));
            DrawCircleV(tip, p->width * 0.6f, WHITE);
        }
    }
}

void DrawShootingEffects(void)
{
    for (int i = 0; i < MAX_SHOCKWAVES; i++)
    {
        if (shockwaves[i].active)
        {
            Shockwave* sw = &shockwaves[i];
            Color ringCol = Fade(sw->color, (1.0f - sw->radius / sw->maxRadius) * 0.85f);
            DrawCircleLines((int)sw->position.x, (int)sw->position.y, sw->radius, ringCol);
            if (sw->thickness > 1.5f)
                DrawCircleLines((int)sw->position.x, (int)sw->position.y, sw->radius + 1.0f, ringCol);
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (particles[i].active)
        {
            VFXParticle* pt = &particles[i];
            Color col = LerpColor(pt->startColor, pt->endColor, 1.0f - pt->timer / pt->maxTimer);
            if (pt->isSmoke)
            {
                DrawCircleV(pt->position, pt->size, col);
            }
            else
            {
                float speed = sqrtf(pt->velocity.x * pt->velocity.x + pt->velocity.y * pt->velocity.y);
                if (speed > 40.0f)
                {
                    float tailLen = fminf(speed * 0.04f, 10.0f);
                    DrawLineEx((Vector2){ pt->position.x - (pt->velocity.x / speed) * tailLen,
                                          pt->position.y - (pt->velocity.y / speed) * tailLen },
                               pt->position, pt->size * 0.8f, col);
                }
                DrawCircleV(pt->position, pt->size * 0.6f, col);
            }
        }
    }

    for (int i = 0; i < MAX_MUZZLE_FLASHES; i++)
    {
        if (muzzleFlashes[i].active)
        {
            MuzzleFlash* mf = &muzzleFlashes[i];
            float progress = mf->timer / mf->maxTimer;
            float curRadius = mf->radius * (0.8f + 0.4f * (1.0f - progress));

            DrawCircleV(mf->position, curRadius * 1.6f, Fade(mf->color, 0.4f * progress));
            DrawCircleV(mf->position, curRadius, Fade(mf->color, 0.85f * progress));
            DrawCircleV(mf->position, curRadius * 0.45f, Fade(WHITE, progress));

            float flareLen = curRadius * 2.2f;
            Color flareCol = Fade(WHITE, 0.75f * progress);
            DrawLineEx((Vector2){ mf->position.x - flareLen, mf->position.y }, (Vector2){ mf->position.x + flareLen, mf->position.y }, 1.5f, flareCol);
            DrawLineEx((Vector2){ mf->position.x, mf->position.y - flareLen }, (Vector2){ mf->position.x, mf->position.y + flareLen }, 1.5f, flareCol);
        }
    }

    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        if (floatTexts[i].active)
        {
            FloatText* ft = &floatTexts[i];
            float alpha = ft->timer / ft->maxTimer;
            int textWidth = MeasureText(ft->text, ft->fontSize);
            int drawX = (int)ft->position.x - textWidth / 2, drawY = (int)ft->position.y;
            DrawText(ft->text, drawX + 1, drawY + 1, ft->fontSize, Fade(BLACK, alpha * 0.8f));
            DrawText(ft->text, drawX, drawY, ft->fontSize, Fade(ft->color, alpha));
        }
    }
}

void TriggerSonicWaveVFX(Vector2 origin)
{
    screenShake = 22.0f;
    Color colors[5] = {
        (Color){ 0, 240, 255, 255 }, WHITE, (Color){ 100, 210, 255, 255 },
        (Color){ 220, 80, 255, 255 }, (Color){ 50, 255, 200, 255 }
    };

    for (int k = 0; k < 5; k++)
        AddShockwave(origin, 12.0f + k * 20.0f, 850.0f, 750.0f + k * 140.0f, 5.5f - k * 0.7f, colors[k]);

    for (int i = 0; i < 120; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(160, 650);
        Color startC = (i % 3 == 0) ? WHITE : ((i % 3 == 1) ? (Color){ 0, 240, 255, 255 } : (Color){ 220, 80, 255, 255 });
        SpawnParticle(origin, (Vector2){ cosf(angle) * speed, sinf(angle) * speed },
                      (float)GetRandomValue(3, 7), (float)GetRandomValue(40, 85) / 100.0f,
                      startC, (Color){ 0, 80, 180, 0 }, 1.6f, false);
    }

    AddFloatText((Vector2){ origin.x, origin.y - 80.0f }, "⚡ SONIC WAVE DETONATED! ⚡", (Color){ 0, 240, 255, 255 }, 24);
    gunship.flameBoost = 40.0f;
    gunship.cannonGlowLeft = 0.9f;
    gunship.cannonGlowRight = 0.9f;
}
