#include "player.h"
#include "game.h"

Player player;

void InitPlayer(void)
{
    player.position = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 60.0f };
    player.width = 0;
    player.height = 0;
}

void UpdatePlayer(void) {}
void DrawPlayer(void) {}

Rectangle GetPlayerRectangle(void)
{
    return (Rectangle){ player.position.x, player.position.y, 0, 0 };
}

void UnloadPlayer(void) {}
