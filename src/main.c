#include "raylib.h"
#include "game.h"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TypingParo");
    SetExitKey(KEY_COMMA); 
    SetTargetFPS(60);

    InitGame();
    
    while (!WindowShouldClose())
    {
        UpdateGame();

        BeginDrawing();
        DrawGame();
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadGame();
    CloseWindow();

    return 0;
}
