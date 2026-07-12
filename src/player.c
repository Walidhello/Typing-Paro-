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

    player.width = 40;
    player.height = 40;
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
    DrawTriangle(

        (Vector2){player.position.x,
                  player.position.y - player.height/2},

        (Vector2){player.position.x - player.width/2,
                  player.position.y + player.height/2},

        (Vector2){player.position.x + player.width/2,
                  player.position.y + player.height/2},

        SKYBLUE
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