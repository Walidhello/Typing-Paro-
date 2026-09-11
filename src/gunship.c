#include "gunship.h"
#include "game.h"
#include "shooting.h"
#include <math.h>

Gunship gunship;

void InitGunship(void)
{
    gunship.width = 180;
    gunship.height = 90;

    gunship.position.x = SCREEN_WIDTH / 2.0f;
    gunship.position.y = SCREEN_HEIGHT - 90;
    gunship.recoilY = 0.0f;
    gunship.cannonGlowLeft = 0.0f;
    gunship.cannonGlowRight = 0.0f;
    gunship.flameBoost = 0.0f;
    gunship.misfireTimer = 0.0f;
}

void UpdateGunship(void)
{
    float dt = GetFrameTime();

    // Smooth recoil spring recovery
    if (gunship.recoilY > 0.0f)
    {
        gunship.recoilY -= 18.0f * dt;
        if (gunship.recoilY < 0.0f) gunship.recoilY = 0.0f;
    }

    // Decay cannon energy glow
    if (gunship.cannonGlowLeft > 0.0f)
    {
        gunship.cannonGlowLeft -= 3.0f * dt;
        if (gunship.cannonGlowLeft < 0.0f) gunship.cannonGlowLeft = 0.0f;
    }
    if (gunship.cannonGlowRight > 0.0f)
    {
        gunship.cannonGlowRight -= 3.0f * dt;
        if (gunship.cannonGlowRight < 0.0f) gunship.cannonGlowRight = 0.0f;
    }

    // Decay engine flame boost
    if (gunship.flameBoost > 0.0f)
    {
        gunship.flameBoost -= 25.0f * dt;
        if (gunship.flameBoost < 0.0f) gunship.flameBoost = 0.0f;
    }

    // Decay misfire indicator
    if (gunship.misfireTimer > 0.0f)
    {
        gunship.misfireTimer -= dt;
        if (gunship.misfireTimer < 0.0f) gunship.misfireTimer = 0.0f;
    }

    // Emit subtle engine thrust particles
    float shipY = gunship.position.y + gunship.recoilY;
    if (GetRandomValue(0, 2) == 0)
    {
        float speed = (float)GetRandomValue(70, 140) + gunship.flameBoost * 2.0f;
        AddExhaustParticle(
            (Vector2){ gunship.position.x - 25.0f + GetRandomValue(-4, 4), shipY + 45.0f },
            (Vector2){ (float)GetRandomValue(-10, 10), speed },
            (Color){ 0, 180, 255, 200 }
        );
        AddExhaustParticle(
            (Vector2){ gunship.position.x + 25.0f + GetRandomValue(-4, 4), shipY + 45.0f },
            (Vector2){ (float)GetRandomValue(-10, 10), speed },
            (Color){ 0, 180, 255, 200 }
        );
    }
}

void DrawGunship(void)
{
    float x = gunship.position.x;
    float y = gunship.position.y + gunship.recoilY;

    // =====================================================
    // DYNAMIC ENGINE FLAMES
    // =====================================================
    float flameFlicker = sinf(GetTime() * 38.0f) * 4.0f + (float)GetRandomValue(-2, 2);
    float flameExtra = gunship.flameBoost + flameFlicker;
    float flameTipY = y + 55.0f + flameExtra;
    float flameCoreTipY = y + 47.0f + flameExtra * 0.75f;

    // Left flame
    DrawTriangle(
        (Vector2){x - 35, y + 25},
        (Vector2){x - 25, flameTipY},
        (Vector2){x - 15, y + 25},
        BLUE
    );

    DrawTriangle(
        (Vector2){x - 31, y + 26},
        (Vector2){x - 25, flameCoreTipY},
        (Vector2){x - 19, y + 26},
        SKYBLUE
    );

    // Right flame
    DrawTriangle(
        (Vector2){x + 35, y + 25},
        (Vector2){x + 15, y + 25},
        (Vector2){x + 25, flameTipY},
        BLUE
    );

    DrawTriangle(
        (Vector2){x + 31, y + 26},
        (Vector2){x + 19, y + 26},
        (Vector2){x + 25, flameCoreTipY},
        SKYBLUE
    );


    // =====================================================
    // ENGINE CORES
    // =====================================================

    DrawCircle(x - 25, y + 22, 9, DARKBLUE);
    DrawCircle(x - 25, y + 22, 5, SKYBLUE);
    DrawCircle(x - 25, y + 22, 2, WHITE);

    DrawCircle(x + 25, y + 22, 9, DARKBLUE);
    DrawCircle(x + 25, y + 22, 5, SKYBLUE);
    DrawCircle(x + 25, y + 22, 2, WHITE);


    // =====================================================
    // CANNONS & BARRELS (Extending forward)
    // =====================================================

    // Left cannon barrel
    DrawRectangle(x - 25, y - 8, 7, 46, DARKGRAY);
    DrawRectangle(x - 24, y - 12, 5, 5, (Color){25, 30, 40, 255}); // Muzzle tip
    Color leftConduitColor = (gunship.cannonGlowLeft > 0.0f) ?
        Fade(SKYBLUE, fminf(gunship.cannonGlowLeft * 3.5f, 1.0f)) : (Color){45, 60, 80, 255};
    DrawRectangle(x - 23, y - 7, 3, 23, leftConduitColor); // Energy conduit

    // Right cannon barrel
    DrawRectangle(x + 18, y - 8, 7, 46, DARKGRAY);
    DrawRectangle(x + 19, y - 12, 5, 5, (Color){25, 30, 40, 255}); // Muzzle tip
    Color rightConduitColor = (gunship.cannonGlowRight > 0.0f) ?
        Fade(SKYBLUE, fminf(gunship.cannonGlowRight * 3.5f, 1.0f)) : (Color){45, 60, 80, 255};
    DrawRectangle(x + 20, y - 7, 3, 23, rightConduitColor); // Energy conduit


    // =====================================================
    // LEFT WING
    // =====================================================

    DrawTriangle(
        (Vector2){x - 12, y - 5},
        (Vector2){x - 90, y + 25},
        (Vector2){x - 28, y + 30},
        (Color){45, 55, 70, 255}
    );

    // Left wing armor
    DrawTriangle(
        (Vector2){x - 18, y - 4},
        (Vector2){x - 75, y + 20},
        (Vector2){x - 30, y + 16},
        (Color){90, 100, 120, 255}
    );

    // Left cyan edge
    DrawLine(
        x - 18, y - 4,
        x - 82, y + 22,
        SKYBLUE
    );


    // =====================================================
    // RIGHT WING
    // =====================================================

    DrawTriangle(
        (Vector2){x + 12, y - 5},
        (Vector2){x + 28, y + 30},
        (Vector2){x + 90, y + 25},
        (Color){45, 55, 70, 255}
    );

    // Right wing armor
    DrawTriangle(
        (Vector2){x + 18, y - 4},
        (Vector2){x + 30, y + 16},
        (Vector2){x + 75, y + 20},
        (Color){90, 100, 120, 255}
    );

    // Right cyan edge
    DrawLine(
        x + 18, y - 4,
        x + 82, y + 22,
        SKYBLUE
    );


    // =====================================================
    // MAIN BODY
    // =====================================================

    // Outer hull
    DrawTriangle(
        (Vector2){x, y - 50},
        (Vector2){x - 36, y + 30},
        (Vector2){x + 36, y + 30},
        (Color){40, 48, 62, 255}
    );

    // Inner hull
    DrawTriangle(
        (Vector2){x, y - 43},
        (Vector2){x - 22, y + 24},
        (Vector2){x + 22, y + 24},
        (Color){80, 90, 105, 255}
    );


    // =====================================================
    // COCKPIT
    // =====================================================

    // Cockpit frame
    DrawTriangle(
        (Vector2){x, y - 37},
        (Vector2){x - 16, y - 4},
        (Vector2){x + 16, y - 4},
        (Color){15, 25, 35, 255}
    );

    // Glass
    DrawTriangle(
        (Vector2){x, y - 32},
        (Vector2){x - 11, y - 7},
        (Vector2){x + 11, y - 7},
        SKYBLUE
    );

    // Reflection
    DrawTriangle(
        (Vector2){x - 2, y - 29},
        (Vector2){x - 7, y - 10},
        (Vector2){x + 2, y - 10},
        WHITE
    );


    // =====================================================
    // NOSE
    // =====================================================

    DrawTriangle(
        (Vector2){x, y - 50},
        (Vector2){x - 7, y - 28},
        (Vector2){x + 7, y - 28},
        (Color){110, 120, 135, 255}
    );

    // Nose light
    DrawRectangle(
        x - 2,
        y - 41,
        4,
        6,
        ORANGE
    );


    // =====================================================
    // WEAPON POD LIGHTS
    // =====================================================

    Color podColor = (gunship.misfireTimer > 0.0f) ? RED : ORANGE;
    DrawRectangle(
        x - 60,
        y + 8,
        6,
        4,
        podColor
    );

    DrawRectangle(
        x + 54,
        y + 8,
        6,
        4,
        podColor
    );


    // =====================================================
    // CENTER DETAIL
    // =====================================================

    DrawLine(
        x,
        y - 5,
        x,
        y + 27,
        LIGHTGRAY
    );
}
