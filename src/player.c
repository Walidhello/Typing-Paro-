#include "player.h"
#include "game.h"

Player player;

//----------------------------------
// Initialize Player
//----------------------------------

void InitPlayer(void)
{
    player.position.x = SCREEN_WIDTH / 2;
    player.position.y = SCREEN_HEIGHT - 60;
    
    // Load your player image here (Ensure you have an image at this path)
    player.texture = LoadTexture("assets/player.png"); 
    
    player.width = player.texture.width;
    player.height = player.texture.height;
}

//----------------------------------
// Update Player
//----------------------------------

void UpdatePlayer(void)
{
    // Player does not move.
}

//----------------------------------
// Draw Player
//----------------------------------

void DrawPlayer(void)
{
    // Draw the image centered on the player's position
    DrawTexture(
        player.texture, 
        player.position.x - player.width / 2, 
        player.position.y - player.height / 2, 
        WHITE
    );
}

//----------------------------------
// Collision Rectangle
//----------------------------------




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