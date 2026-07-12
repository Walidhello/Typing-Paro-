#include "raylib.h"
#include "game.h"

int main(void)
{
    // Create Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TypingParo");

    // Run at 60 FPS
    SetTargetFPS(60);

    // Initialize Game
    InitGame();

    // Main Game Loop
    while (!WindowShouldClose())
    {
        // Update
        UpdateGame();

        // Draw
        BeginDrawing();

        DrawGame();

        DrawFPS(10,10);

        EndDrawing();
    }

    // Free Memory
    UnloadGame();

    // Close Window
    CloseWindow();

    return 0;
}