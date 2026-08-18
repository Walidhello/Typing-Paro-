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

/*void DrawGunship(void)
{
    float x = gunship.position.x;
    float y = gunship.position.y;

    // =====================================================
    // ENGINE FLAMES
    // =====================================================

    // Left engine flame
    DrawTriangle(
        (Vector2){x - 32, y + 24},
        (Vector2){x - 24, y + 55},
        (Vector2){x - 16, y + 24},
        BLUE
    );

    DrawTriangle(
        (Vector2){x - 29, y + 24},
        (Vector2){x - 24, y + 46},
        (Vector2){x - 20, y + 24},
        SKYBLUE
    );

    // Right engine flame
    DrawTriangle(
        (Vector2){x + 32, y + 24},
        (Vector2){x + 24, y + 55},
        (Vector2){x + 16, y + 24},
        BLUE
    );

    DrawTriangle(
        (Vector2){x + 29, y + 24},
        (Vector2){x + 24, y + 46},
        (Vector2){x + 20, y + 24},
        SKYBLUE
    );


    // =====================================================
    // ENGINE GLOW
    // =====================================================

    DrawCircle(x - 24, y + 20, 10, DARKBLUE);
    DrawCircle(x - 24, y + 20, 6, SKYBLUE);
    DrawCircle(x - 24, y + 20, 2, WHITE);

    DrawCircle(x + 24, y + 20, 10, DARKBLUE);
    DrawCircle(x + 24, y + 20, 6, SKYBLUE);
    DrawCircle(x + 24, y + 20, 2, WHITE);


    // =====================================================
    // LEFT WING
    // =====================================================

    DrawTriangle(
        (Vector2){x - 15, y - 5},
        (Vector2){x - 85, y + 25},
        (Vector2){x - 28, y + 25},
        (Color){50, 60, 75, 255}
    );

    // Left wing upper panel
    DrawTriangle(
        (Vector2){x - 20, y - 7},
        (Vector2){x - 72, y + 18},
        (Vector2){x - 32, y + 16},
        (Color){85, 95, 110, 255}
    );

    // Left wing edge
    DrawLine(
        x - 20,
        y - 7,
        x - 78,
        y + 20,
        SKYBLUE
    );


    // =====================================================
    // RIGHT WING
    // =====================================================

    DrawTriangle(
        (Vector2){x + 15, y - 5},
        (Vector2){x + 85, y + 25},
        (Vector2){x + 28, y + 25},
        (Color){50, 60, 75, 255}
    );

    // Right wing upper panel
    DrawTriangle(
        (Vector2){x + 20, y - 7},
        (Vector2){x + 72, y + 18},
        (Vector2){x + 32, y + 16},
        (Color){85, 95, 110, 255}
    );

    // Right wing edge
    DrawLine(
        x + 20,
        y - 7,
        x + 78,
        y + 20,
        SKYBLUE
    );


    // =====================================================
    // MAIN BODY
    // =====================================================

    DrawTriangle(
        (Vector2){x, y - 48},
        (Vector2){x - 35, y + 28},
        (Vector2){x + 35, y + 28},
        (Color){40, 48, 62, 255}
    );

    // Center armor
    DrawTriangle(
        (Vector2){x, y - 42},
        (Vector2){x - 20, y + 22},
        (Vector2){x + 20, y + 22},
        (Color){75, 85, 100, 255}
    );


    // =====================================================
    // COCKPIT
    // =====================================================

    DrawTriangle(
        (Vector2){x, y - 35},
        (Vector2){x - 15, y - 3},
        (Vector2){x + 15, y - 3},
        (Color){15, 25, 35, 255}
    );

    DrawTriangle(
        (Vector2){x, y - 30},
        (Vector2){x - 10, y - 6},
        (Vector2){x + 10, y - 6},
        SKYBLUE
    );

    DrawTriangle(
        (Vector2){x - 2, y - 27},
        (Vector2){x - 7, y - 8},
        (Vector2){x + 2, y - 8},
        WHITE
    );


    // =====================================================
    // NOSE
    // =====================================================

    DrawTriangle(
        (Vector2){x, y - 48},
        (Vector2){x - 7, y - 28},
        (Vector2){x + 7, y - 28},
        (Color){110, 120, 135, 255}
    );

    DrawRectangle(
        x - 2,
        y - 39,
        4,
        6,
        ORANGE
    );


    // =====================================================
    // WEAPON PODS
    // =====================================================

    // Left
    DrawRectangle(
        x - 62,
        y + 9,
        20,
        7,
        (Color){30, 35, 45, 255}
    );

    DrawRectangle(
        x - 65,
        y + 10,
        5,
        4,
        ORANGE
    );

    // Right
    DrawRectangle(
        x + 42,
        y + 9,
        20,
        7,
        (Color){30, 35, 45, 255}
    );

    DrawRectangle(
        x + 60,
        y + 10,
        5,
        4,
        ORANGE
    );


    // =====================================================
    // FRONT CANNONS
    // =====================================================

    DrawRectangle(
        x - 22,
        y + 18,
        7,
        22,
        DARKGRAY
    );

    DrawRectangle(
        x + 15,
        y + 18,
        7,
        22,
        DARKGRAY
    );


    // =====================================================
    // ENGINE HOUSINGS
    // =====================================================

    DrawRectangle(
        x - 35,
        y + 16,
        14,
        14,
        (Color){25, 30, 40, 255}
    );

    DrawRectangle(
        x + 21,
        y + 16,
        14,
        14,
        (Color){25, 30, 40, 255}
    );


    // =====================================================
    // CENTRAL ARMOR LINE
    // =====================================================

    DrawLine(
        x,
        y - 5,
        x,
        y + 27,
        LIGHTGRAY
    );
}
void DrawGunship(void)
{
    float x = gunship.position.x;
    float y = gunship.position.y;

    // Center marker
    DrawRectangle(x - 25, y - 25, 50, 50, WHITE);

    // LEFT marker
    DrawRectangle(x - 150, y - 25, 50, 50, RED);

    // RIGHT marker
    DrawRectangle(x + 100, y - 25, 50, 50, GREEN);

    // Center line
    DrawLine(x, 0, x, SCREEN_HEIGHT, YELLOW);
}*/
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