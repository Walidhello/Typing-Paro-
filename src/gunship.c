#include "gunship.h"
#include "game.h"
#include "enemy.h"
#include "shooting.h"
#include "rlgl.h"
#include <math.h>

#define PI_FLOAT 3.14159265358979323846f

Gunship gunship;

void InitGunship(void)
{
    gunship.width = 180;
    gunship.height = 90;

    gunship.position.x = SCREEN_WIDTH / 2.0f;
    gunship.position.y = SCREEN_HEIGHT - 90;
    gunship.rotation = 0.0f;
    gunship.targetRotation = 0.0f;
    gunship.recoilY = 0.0f;
    gunship.cannonGlowLeft = 0.0f;
    gunship.cannonGlowRight = 0.0f;
    gunship.flameBoost = 0.0f;
    gunship.misfireTimer = 0.0f;
}

void UpdateGunship(void)
{
    float dt = GetFrameTime();

    float targetAngle = 0.0f;

    if (targetEnemy >= 0 && targetEnemy < MAX_ENEMIES && enemies[targetEnemy].active)
    {
        float dx = enemies[targetEnemy].x - gunship.position.x;
        float dy = enemies[targetEnemy].y - (gunship.position.y + gunship.recoilY);

        // Angle towards target where 0 deg is straight up
        targetAngle = atan2f(dy, dx) * (180.0f / PI_FLOAT) + 90.0f;
    }

    gunship.targetRotation = targetAngle;

    // Shortest-path angle difference (-180 to 180 wrap)
    float diff = gunship.targetRotation - gunship.rotation;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;

    float turnSpeed = 14.0f;
    gunship.rotation += diff * (1.0f - expf(-turnSpeed * dt));

    if (gunship.recoilY > 0.0f)
    {
        gunship.recoilY -= 18.0f * dt;
        if (gunship.recoilY < 0.0f) gunship.recoilY = 0.0f;
    }

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

    if (gunship.flameBoost > 0.0f)
    {
        gunship.flameBoost -= 25.0f * dt;
        if (gunship.flameBoost < 0.0f) gunship.flameBoost = 0.0f;
    }

    if (gunship.misfireTimer > 0.0f)
    {
        gunship.misfireTimer -= dt;
        if (gunship.misfireTimer < 0.0f) gunship.misfireTimer = 0.0f;
    }

    float shipY = gunship.position.y + gunship.recoilY;
    if (GetRandomValue(0, 2) == 0)
    {
        float speed = (float)GetRandomValue(70, 140) + gunship.flameBoost * 2.0f;
        float rad = gunship.rotation * (PI_FLOAT / 180.0f);
        float cosA = cosf(rad);
        float sinA = sinf(rad);

        Vector2 leftNozzle = {
            gunship.position.x + (-25.0f * cosA - 45.0f * sinA),
            shipY + (-25.0f * sinA + 45.0f * cosA)
        };
        Vector2 rightNozzle = {
            gunship.position.x + (25.0f * cosA - 45.0f * sinA),
            shipY + (25.0f * sinA + 45.0f * cosA)
        };

        Vector2 backDir = { -sinA, cosA };
        Vector2 exhaustVelL = {
            backDir.x * speed + (float)GetRandomValue(-10, 10),
            backDir.y * speed + (float)GetRandomValue(-5, 5)
        };
        Vector2 exhaustVelR = {
            backDir.x * speed + (float)GetRandomValue(-10, 10),
            backDir.y * speed + (float)GetRandomValue(-5, 5)
        };

        AddExhaustParticle(leftNozzle, exhaustVelL, (Color){ 0, 180, 255, 200 });
        AddExhaustParticle(rightNozzle, exhaustVelR, (Color){ 0, 180, 255, 200 });
    }
}

Vector2 GetGunshipLeftMuzzle(void)
{
    float rad = gunship.rotation * (PI_FLOAT / 180.0f);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float localX = -21.5f;
    float localY = -12.0f;
    float shipY = gunship.position.y + gunship.recoilY;

    return (Vector2){
        gunship.position.x + (localX * cosA - localY * sinA),
        shipY + (localX * sinA + localY * cosA)
    };
}

Vector2 GetGunshipRightMuzzle(void)
{
    float rad = gunship.rotation * (PI_FLOAT / 180.0f);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float localX = 21.5f;
    float localY = -12.0f;
    float shipY = gunship.position.y + gunship.recoilY;

    return (Vector2){
        gunship.position.x + (localX * cosA - localY * sinA),
        shipY + (localX * sinA + localY * cosA)
    };
}

void DrawGunship(void)
{
    float x = gunship.position.x;
    float y = gunship.position.y + gunship.recoilY;

    rlPushMatrix();
    rlTranslatef(x, y, 0.0f);
    rlRotatef(gunship.rotation, 0.0f, 0.0f, 1.0f);
    rlTranslatef(-x, -y, 0.0f);

    float flameFlicker = sinf(GetTime() * 38.0f) * 4.0f + (float)GetRandomValue(-2, 2);
    float flameExtra = gunship.flameBoost + flameFlicker;
    float flameTipY = y + 55.0f + flameExtra;
    float flameCoreTipY = y + 47.0f + flameExtra * 0.75f;

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

    DrawCircle(x - 25, y + 22, 9, DARKBLUE);
    DrawCircle(x - 25, y + 22, 5, SKYBLUE);
    DrawCircle(x - 25, y + 22, 2, WHITE);

    DrawCircle(x + 25, y + 22, 9, DARKBLUE);
    DrawCircle(x + 25, y + 22, 5, SKYBLUE);
    DrawCircle(x + 25, y + 22, 2, WHITE);

    DrawRectangle(x - 25, y - 8, 7, 46, DARKGRAY);
    DrawRectangle(x - 24, y - 12, 5, 5, (Color){25, 30, 40, 255});
    Color leftConduitColor = (gunship.cannonGlowLeft > 0.0f) ?
        Fade(SKYBLUE, fminf(gunship.cannonGlowLeft * 3.5f, 1.0f)) : (Color){45, 60, 80, 255};
    DrawRectangle(x - 23, y - 7, 3, 23, leftConduitColor);

    DrawRectangle(x + 18, y - 8, 7, 46, DARKGRAY);
    DrawRectangle(x + 19, y - 12, 5, 5, (Color){25, 30, 40, 255});
    Color rightConduitColor = (gunship.cannonGlowRight > 0.0f) ?
        Fade(SKYBLUE, fminf(gunship.cannonGlowRight * 3.5f, 1.0f)) : (Color){45, 60, 80, 255};
    DrawRectangle(x + 20, y - 7, 3, 23, rightConduitColor);

    DrawTriangle(
        (Vector2){x - 12, y - 5},
        (Vector2){x - 90, y + 25},
        (Vector2){x - 28, y + 30},
        (Color){45, 55, 70, 255}
    );

    DrawTriangle(
        (Vector2){x - 18, y - 4},
        (Vector2){x - 75, y + 20},
        (Vector2){x - 30, y + 16},
        (Color){90, 100, 120, 255}
    );

    DrawLine(
        x - 18, y - 4,
        x - 82, y + 22,
        SKYBLUE
    );

    DrawTriangle(
        (Vector2){x + 12, y - 5},
        (Vector2){x + 28, y + 30},
        (Vector2){x + 90, y + 25},
        (Color){45, 55, 70, 255}
    );

    DrawTriangle(
        (Vector2){x + 18, y - 4},
        (Vector2){x + 30, y + 16},
        (Vector2){x + 75, y + 20},
        (Color){90, 100, 120, 255}
    );

    DrawLine(
        x + 18, y - 4,
        x + 82, y + 22,
        SKYBLUE
    );

    DrawTriangle(
        (Vector2){x, y - 50},
        (Vector2){x - 36, y + 30},
        (Vector2){x + 36, y + 30},
        (Color){40, 48, 62, 255}
    );

    DrawTriangle(
        (Vector2){x, y - 43},
        (Vector2){x - 22, y + 24},
        (Vector2){x + 22, y + 24},
        (Color){80, 90, 105, 255}
    );

    DrawTriangle(
        (Vector2){x, y - 37},
        (Vector2){x - 16, y - 4},
        (Vector2){x + 16, y - 4},
        (Color){15, 25, 35, 255}
    );

    DrawTriangle(
        (Vector2){x, y - 32},
        (Vector2){x - 11, y - 7},
        (Vector2){x + 11, y - 7},
        SKYBLUE
    );

    DrawTriangle(
        (Vector2){x - 2, y - 29},
        (Vector2){x - 7, y - 10},
        (Vector2){x + 2, y - 10},
        WHITE
    );

    DrawTriangle(
        (Vector2){x, y - 50},
        (Vector2){x - 7, y - 28},
        (Vector2){x + 7, y - 28},
        (Color){110, 120, 135, 255}
    );

    DrawRectangle(
        x - 2,
        y - 41,
        4,
        6,
        ORANGE
    );

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

    DrawLine(
        x,
        y - 5,
        x,
        y + 27,
        LIGHTGRAY
    );

    rlPopMatrix();
}
