#include "player.h"
#include "game.h"

Player player;

void InitPlayer(void)
{
    player.position.x = SCREEN_WIDTH / 2;
    player.position.y = SCREEN_HEIGHT - 60;
    
    player.texture = LoadTexture("assets/player.png"); 
    
    player.width = player.texture.width;
    player.height = player.texture.height;
}

void UpdatePlayer(void)
{
    // Player does not move.
}

void DrawPlayer(void)
{
    DrawTexture(
        player.texture, 
        player.position.x - player.width / 2, 
        player.position.y - player.height / 2, 
        WHITE
    );
}

Rectangle GetPlayerRectangle(void)
{
    return (Rectangle)
    {
        player.position.x - player.width/2,
        player.position.y - player.height/2,
        player.width,
        player.height
    };
}

void UnloadPlayer(void)
{
    UnloadTexture(player.texture);
}