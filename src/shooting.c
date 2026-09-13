#include "shooting.h"
#include "gunship.h"
#include "enemy.h"
#include "game.h"
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
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
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

    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        projectiles[i].active = false;
    }
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].active = false;
    }
    for (int i = 0; i < MAX_MUZZLE_FLASHES; i++)
    {
        muzzleFlashes[i].active = false;
    }
    for (int i = 0; i < MAX_SHOCKWAVES; i++)
    {
        shockwaves[i].active = false;
    }
    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        floatTexts[i].active = false;
    }
}

void AddMuzzleFlash(Vector2 pos, Color color)
{
    for (int i = 0; i < MAX_MUZZLE_FLASHES; i++)
    {
        if (!muzzleFlashes[i].active)
        {
            muzzleFlashes[i].active = true;
            muzzleFlashes[i].position = pos;
            muzzleFlashes[i].timer = 0.09f;
            muzzleFlashes[i].maxTimer = 0.09f;
            muzzleFlashes[i].radius = 12.0f;
            muzzleFlashes[i].color = color;
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
            particles[i].active = true;
            particles[i].position = pos;
            particles[i].velocity = vel;
            particles[i].size = size;
            particles[i].timer = lifetime;
            particles[i].maxTimer = lifetime;
            particles[i].startColor = startCol;
            particles[i].endColor = endCol;
            particles[i].drag = drag;
            particles[i].isSmoke = isSmoke;
            break;
        }
    }
}

void AddExhaustParticle(Vector2 pos, Vector2 vel, Color color)
{
    SpawnParticle(
        pos,
        vel,
        (float)GetRandomValue(3, 6),
        0.18f,
        color,
        (Color){ color.r / 3, color.g / 3, color.b / 3, 0 },
        0.5f,
        false
    );
}

void CreateHitSparks(Vector2 pos, Color color, int count)
{
    for (int i = 0; i < count; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(120, 380);
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        float size = (float)GetRandomValue(2, 4);
        float life = (float)GetRandomValue(12, 25) / 100.0f;

        SpawnParticle(
            pos,
            vel,
            size,
            life,
            WHITE,
            Fade(color, 0.0f),
            4.0f,
            false
        );
    }

    for (int i = 0; i < MAX_SHOCKWAVES; i++)
    {
        if (!shockwaves[i].active)
        {
            shockwaves[i].active = true;
            shockwaves[i].position = pos;
            shockwaves[i].radius = 3.0f;
            shockwaves[i].maxRadius = 24.0f;
            shockwaves[i].speed = 160.0f;
            shockwaves[i].thickness = 2.0f;
            shockwaves[i].color = color;
            break;
        }
    }
}

void AddFloatText(Vector2 pos, const char* text, Color color, int fontSize)
{
    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        if (!floatTexts[i].active)
        {
            floatTexts[i].active = true;
            floatTexts[i].position = pos;
            strncpy(floatTexts[i].text, text, sizeof(floatTexts[i].text) - 1);
            floatTexts[i].text[sizeof(floatTexts[i].text) - 1] = '\0';
            floatTexts[i].timer = 0.85f;
            floatTexts[i].maxTimer = 0.85f;
            floatTexts[i].color = color;
            floatTexts[i].fontSize = fontSize;
            break;
        }
    }
}

void CreateExplosion(Vector2 pos, EnemyType type, const char* word)
{
    int particleCount = 40;
    float baseSpeed = 240.0f;
    float shockwaveMax = 70.0f;
    Color primaryColor = (Color){ 255, 120, 30, 255 };
    Color secondaryColor = (Color){ 255, 220, 50, 255 };

    if (type == E_MEDIUM_BOSS)
    {
        particleCount = 65;
        baseSpeed = 320.0f;
        shockwaveMax = 95.0f;
        primaryColor = (Color){ 255, 180, 20, 255 };
        secondaryColor = (Color){ 255, 240, 120, 255 };
        screenShake = fmaxf(screenShake, 7.5f);
    }
    else if (type == E_HARD_BOSS)
    {
        particleCount = 90;
        baseSpeed = 400.0f;
        shockwaveMax = 125.0f;
        primaryColor = (Color){ 220, 60, 255, 255 };
        secondaryColor = (Color){ 80, 220, 255, 255 };
        screenShake = fmaxf(screenShake, 11.0f);
    }
    else
    {
        screenShake = fmaxf(screenShake, 4.5f);
    }

    for (int i = 0; i < particleCount / 3; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(40, (int)(baseSpeed * 0.6f));
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        float size = (float)GetRandomValue(6, 14);
        float life = (float)GetRandomValue(35, 65) / 100.0f;

        SpawnParticle(
            pos,
            vel,
            size,
            life,
            secondaryColor,
            (Color){ 40, 35, 45, 0 },
            2.5f,
            true
        );
    }

    for (int i = 0; i < particleCount; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(100, (int)baseSpeed);
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        float size = (float)GetRandomValue(2, 5);
        float life = (float)GetRandomValue(25, 55) / 100.0f;

        Color startC = (i % 2 == 0) ? WHITE : primaryColor;
        SpawnParticle(
            pos,
            vel,
            size,
            life,
            startC,
            Fade(secondaryColor, 0.0f),
            3.2f,
            false
        );
    }

    for (int i = 0; i < MAX_SHOCKWAVES; i++)
    {
        if (!shockwaves[i].active)
        {
            shockwaves[i].active = true;
            shockwaves[i].position = pos;
            shockwaves[i].radius = 8.0f;
            shockwaves[i].maxRadius = shockwaveMax;
            shockwaves[i].speed = 280.0f;
            shockwaves[i].thickness = 3.5f;
            shockwaves[i].color = secondaryColor;
            break;
        }
    }

    if (type != E_NORMAL)
    {
        for (int i = 0; i < MAX_SHOCKWAVES; i++)
        {
            if (!shockwaves[i].active)
            {
                shockwaves[i].active = true;
                shockwaves[i].position = pos;
                shockwaves[i].radius = 4.0f;
                shockwaves[i].maxRadius = shockwaveMax * 1.3f;
                shockwaves[i].speed = 360.0f;
                shockwaves[i].thickness = 2.0f;
                shockwaves[i].color = primaryColor;
                break;
            }
        }
    }

    if (word != NULL && strlen(word) > 0)
    {
        char buffer[40];
        snprintf(buffer, sizeof(buffer), "+%s", word);
        AddFloatText(
            (Vector2){ pos.x, pos.y - 25.0f },
            buffer,
            (type == E_NORMAL) ? (Color){ 255, 230, 80, 255 } : (Color){ 255, 100, 220, 255 },
            20
        );
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
            projectiles[i].active = true;
            projectiles[i].position = origin;
            projectiles[i].target = target;
            projectiles[i].isKillShot = isKill;
            projectiles[i].enemyType = enemyType;
            if (word) strncpy(projectiles[i].targetWord, word, sizeof(projectiles[i].targetWord) - 1);
            else projectiles[i].targetWord[0] = '\0';
            projectiles[i].coreColor = coreCol;
            projectiles[i].glowColor = glowCol;
            projectiles[i].speed = speed;
            projectiles[i].length = length;
            projectiles[i].width = width;
            projectiles[i].trailCount = 0;

            float dx = target.x - origin.x;
            float dy = target.y - origin.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 0.001f)
            {
                projectiles[i].velocity = (Vector2){ (dx / dist) * speed, (dy / dist) * speed };
            }
            else
            {
                projectiles[i].velocity = (Vector2){ 0.0f, -speed };
            }

            for (int t = 0; t < 6; t++)
            {
                projectiles[i].trail[t] = origin;
            }
            projectiles[i].trailCount = 1;
            break;
        }
    }
}

void FireLaserAtPosition(Vector2 targetPos, EnemyType enemyType, const char* word, bool isKillShot)
{
    if (!isKillShot)
    {
        // Alternate between left and right cannons
        Vector2 muzzlePos;
        if (lastCannon == 0)
        {
            muzzlePos = GetGunshipLeftMuzzle();
            gunship.cannonGlowLeft = 0.25f;
            lastCannon = 1;
        }
        else
        {
            muzzlePos = GetGunshipRightMuzzle();
            gunship.cannonGlowRight = 0.25f;
            lastCannon = 0;
        }

        AddMuzzleFlash(muzzlePos, (Color){ 0, 240, 255, 255 });

        gunship.recoilY = 3.2f;
        gunship.flameBoost = 12.0f;

        SpawnProjectile(
            muzzlePos,
            targetPos,
            false,
            enemyType,
            word,
            (Color){ 240, 255, 255, 255 },
            (Color){ 0, 220, 255, 200 },
            3000.0f,
            26.0f,
            4.0f
        );
    }
    else
    {
        Vector2 leftMuzzle = GetGunshipLeftMuzzle();
        Vector2 rightMuzzle = GetGunshipRightMuzzle();

        gunship.cannonGlowLeft = 0.45f;
        gunship.cannonGlowRight = 0.45f;

        Color glowCol;
        if (enemyType == E_HARD_BOSS)
        {
            glowCol = (Color){ 255, 80, 240, 240 };
        }
        else if (enemyType == E_MEDIUM_BOSS)
        {
            glowCol = (Color){ 255, 190, 40, 240 };
        }
        else
        {
            glowCol = (Color){ 40, 255, 200, 240 };
        }

        AddMuzzleFlash(leftMuzzle, glowCol);
        AddMuzzleFlash(rightMuzzle, glowCol);

        gunship.recoilY = 6.0f;
        gunship.flameBoost = 22.0f;

        SpawnProjectile(
            leftMuzzle,
            targetPos,
            true,
            enemyType,
            word,
            WHITE,
            glowCol,
            3400.0f,
            38.0f,
            6.0f
        );

        SpawnProjectile(
            rightMuzzle,
            targetPos,
            false,
            enemyType,
            word,
            WHITE,
            glowCol,
            3400.0f,
            38.0f,
            6.0f
        );
    }
}

void TriggerMisfire(void)
{
    gunship.misfireTimer = 0.15f;
    float shipY = gunship.position.y + gunship.recoilY;

    SpawnParticle(
        (Vector2){ gunship.position.x - 57.0f, shipY + 10.0f },
        (Vector2){ (float)GetRandomValue(-40, -10), (float)GetRandomValue(-20, 20) },
        4.0f,
        0.15f,
        RED,
        Fade(DARKGRAY, 0.0f),
        2.0f,
        true
    );
    SpawnParticle(
        (Vector2){ gunship.position.x + 57.0f, shipY + 10.0f },
        (Vector2){ (float)GetRandomValue(10, 40), (float)GetRandomValue(-20, 20) },
        4.0f,
        0.15f,
        RED,
        Fade(DARKGRAY, 0.0f),
        2.0f,
        true
    );
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
            {
                p->trail[t] = p->trail[t - 1];
            }
            p->trail[0] = p->position;

            float dx = p->target.x - p->position.x;
            float dy = p->target.y - p->position.y;
            float dist = sqrtf(dx * dx + dy * dy);

            float step = p->speed * dt;

            if (dist <= step || dist < 25.0f)
            {
                p->active = false;
                Vector2 impactPoint = p->target;

                if (!p->isKillShot)
                {
                    CreateHitSparks(impactPoint, p->glowColor, 9);
                }
                else
                {
                    CreateExplosion(impactPoint, p->enemyType, p->targetWord);
                }
            }
            else
            {
                p->velocity.x = (dx / dist) * p->speed;
                p->velocity.y = (dy / dist) * p->speed;

                p->position.x += p->velocity.x * dt;
                p->position.y += p->velocity.y * dt;

                if (p->position.y < -120.0f || p->position.y > SCREEN_HEIGHT + 60.0f ||
                    p->position.x < -100.0f || p->position.x > SCREEN_WIDTH + 100.0f)
                {
                    p->active = false;
                }
            }
        }
    }

    for (int i = 0; i < MAX_MUZZLE_FLASHES; i++)
    {
        if (muzzleFlashes[i].active)
        {
            muzzleFlashes[i].timer -= dt;
            if (muzzleFlashes[i].timer <= 0.0f)
            {
                muzzleFlashes[i].active = false;
            }
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (particles[i].active)
        {
            VFXParticle* pt = &particles[i];
            pt->timer -= dt;

            if (pt->timer <= 0.0f)
            {
                pt->active = false;
            }
            else
            {
                pt->velocity.x *= (1.0f - pt->drag * dt);
                pt->velocity.y *= (1.0f - pt->drag * dt);

                pt->position.x += pt->velocity.x * dt;
                pt->position.y += pt->velocity.y * dt;

                if (pt->isSmoke)
                {
                    pt->size += 7.0f * dt;
                }
            }
        }
    }

    for (int i = 0; i < MAX_SHOCKWAVES; i++)
    {
        if (shockwaves[i].active)
        {
            shockwaves[i].radius += shockwaves[i].speed * dt;
            if (shockwaves[i].radius >= shockwaves[i].maxRadius)
            {
                shockwaves[i].active = false;
            }
        }
    }

    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        if (floatTexts[i].active)
        {
            floatTexts[i].timer -= dt;
            floatTexts[i].position.y -= 32.0f * dt;

            if (floatTexts[i].timer <= 0.0f)
            {
                floatTexts[i].active = false;
            }
        }
    }

    if (screenShake > 0.0f)
    {
        screenShake -= 24.0f * dt;
        if (screenShake < 0.0f) screenShake = 0.0f;

        shakeOffset.x = ((float)GetRandomValue(-100, 100) / 100.0f) * screenShake;
        shakeOffset.y = ((float)GetRandomValue(-100, 100) / 100.0f) * screenShake;
    }
    else
    {
        shakeOffset = (Vector2){ 0.0f, 0.0f };
    }
}

Vector2 GetScreenShakeOffset(void)
{
    return shakeOffset;
}

void DrawShootingProjectiles(void)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            LaserProjectile* p = &projectiles[i];

            float dx = p->velocity.x;
            float dy = p->velocity.y;
            float len = sqrtf(dx * dx + dy * dy);
            if (len < 0.001f) continue;

            Vector2 dir = { dx / len, dy / len };
            Vector2 tip = p->position;
            Vector2 tail = { tip.x - dir.x * p->length, tip.y - dir.y * p->length };

            for (int t = 0; t < p->trailCount - 1; t++)
            {
                float trailProgress = 1.0f - ((float)t / (float)p->trailCount);
                Color trailCol = Fade(p->glowColor, 0.35f * trailProgress);
                DrawLineEx(p->trail[t], p->trail[t + 1], p->width * 0.8f * trailProgress, trailCol);
            }

            DrawLineEx(tail, tip, p->width * 2.6f, Fade(p->glowColor, 0.35f));

            DrawLineEx(tail, tip, p->width * 1.5f, Fade(p->glowColor, 0.85f));

            Vector2 coreTail = { tip.x - dir.x * (p->length * 0.7f), tip.y - dir.y * (p->length * 0.7f) };
            DrawLineEx(coreTail, tip, p->width * 0.6f, p->coreColor);

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
            float alpha = 1.0f - (sw->radius / sw->maxRadius);
            Color ringCol = Fade(sw->color, alpha * 0.85f);

            DrawCircleLines((int)sw->position.x, (int)sw->position.y, sw->radius, ringCol);
            if (sw->thickness > 1.5f)
            {
                DrawCircleLines((int)sw->position.x, (int)sw->position.y, sw->radius + 1.0f, ringCol);
            }
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (particles[i].active)
        {
            VFXParticle* pt = &particles[i];
            float lifeProgress = 1.0f - (pt->timer / pt->maxTimer);
            Color col = LerpColor(pt->startColor, pt->endColor, lifeProgress);

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
                    Vector2 tail = {
                        pt->position.x - (pt->velocity.x / speed) * tailLen,
                        pt->position.y - (pt->velocity.y / speed) * tailLen
                    };
                    DrawLineEx(tail, pt->position, pt->size * 0.8f, col);
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
            DrawLineEx(
                (Vector2){ mf->position.x - flareLen, mf->position.y },
                (Vector2){ mf->position.x + flareLen, mf->position.y },
                1.5f,
                flareCol
            );
            DrawLineEx(
                (Vector2){ mf->position.x, mf->position.y - flareLen },
                (Vector2){ mf->position.x, mf->position.y + flareLen },
                1.5f,
                flareCol
            );
        }
    }

    for (int i = 0; i < MAX_FLOAT_TEXTS; i++)
    {
        if (floatTexts[i].active)
        {
            FloatText* ft = &floatTexts[i];
            float alpha = ft->timer / ft->maxTimer;
            int textWidth = MeasureText(ft->text, ft->fontSize);
            int drawX = (int)ft->position.x - textWidth / 2;
            int drawY = (int)ft->position.y;

            DrawText(ft->text, drawX + 1, drawY + 1, ft->fontSize, Fade(BLACK, alpha * 0.8f));
            DrawText(ft->text, drawX, drawY, ft->fontSize, Fade(ft->color, alpha));
        }
    }
}

void TriggerSonicWaveVFX(Vector2 origin)
{
    screenShake = 22.0f;

    Color colors[5] = {
        (Color){ 0, 240, 255, 255 },
        WHITE,
        (Color){ 100, 210, 255, 255 },
        (Color){ 220, 80, 255, 255 },
        (Color){ 50, 255, 200, 255 }
    };

    for (int k = 0; k < 5; k++)
    {
        for (int i = 0; i < MAX_SHOCKWAVES; i++)
        {
            if (!shockwaves[i].active)
            {
                shockwaves[i].active = true;
                shockwaves[i].position = origin;
                shockwaves[i].radius = 12.0f + k * 20.0f;
                shockwaves[i].maxRadius = 850.0f;
                shockwaves[i].speed = 750.0f + k * 140.0f;
                shockwaves[i].thickness = 5.5f - k * 0.7f;
                shockwaves[i].color = colors[k];
                break;
            }
        }
    }

    for (int i = 0; i < 120; i++)
    {
        float angle = ((float)GetRandomValue(0, 360)) * (PI_FLOAT / 180.0f);
        float speed = (float)GetRandomValue(160, 650);
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        float size = (float)GetRandomValue(3, 7);
        float life = (float)GetRandomValue(40, 85) / 100.0f;

        Color startC = (i % 3 == 0) ? WHITE : ((i % 3 == 1) ? (Color){ 0, 240, 255, 255 } : (Color){ 220, 80, 255, 255 });
        SpawnParticle(
            origin,
            vel,
            size,
            life,
            startC,
            (Color){ 0, 80, 180, 0 },
            1.6f,
            false
        );
    }

    AddFloatText(
        (Vector2){ origin.x, origin.y - 80.0f },
        "⚡ SONIC WAVE DETONATED! ⚡",
        (Color){ 0, 240, 255, 255 },
        24
    );

    gunship.flameBoost = 40.0f;
    gunship.cannonGlowLeft = 0.9f;
    gunship.cannonGlowRight = 0.9f;
}
