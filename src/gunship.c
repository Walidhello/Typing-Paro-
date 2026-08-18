#include "gunship.h"
#include "game.h"

Gunship gunship;

void InitGunship(void)
{
    gunship.width = 180;
    gunship.height = 90;

    gunship.position.x = SCREEN_WIDTH / 2.0f;
    gunship.position.y = SCREEN_HEIGHT - 90;
}
void UpdateGunship(void)
{
    // Gunship currently stays stationary.
}

void DrawGunship(void)
{
    float x = gunship.position.x;
    float y = gunship.position.y;

    // =====================================================
    // ENGINE FLAMES
    // =====================================================

    // Left flame
    DrawTriangle(
        (Vector2){x - 35, y + 25},
        (Vector2){x - 25, y + 55},
        (Vector2){x - 15, y + 25},
        BLUE
    );

    DrawTriangle(
        (Vector2){x - 31, y + 26},
        (Vector2){x - 25, y + 47},
        (Vector2){x - 19, y + 26},
        SKYBLUE
    );

    // Right flame
    DrawTriangle(
        (Vector2){x + 35, y + 25},
        (Vector2){x + 15, y + 25},
        (Vector2){x + 25, y + 55},
        BLUE
    );

    DrawTriangle(
        (Vector2){x + 31, y + 26},
        (Vector2){x + 19, y + 26},
        (Vector2){x + 25, y + 47},
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
    // CANNONS
    // =====================================================

    // Left cannon
    DrawRectangle(
        x - 25,
        y + 16,
        7,
        22,
        DARKGRAY
    );

    // Right cannon
    DrawRectangle(
        x + 18,
        y + 16,
        7,
        22,
        DARKGRAY
    );


    // =====================================================
    // WEAPON POD LIGHTS
    // =====================================================

    DrawRectangle(
        x - 60,
        y + 8,
        6,
        4,
        ORANGE
    );

    DrawRectangle(
        x + 54,
        y + 8,
        6,
        4,
        ORANGE
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