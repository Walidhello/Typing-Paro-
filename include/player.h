#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

//----------------------------------
// Player Structure
//----------------------------------
typedef struct
{
    Vector2 position;
    int width;
    int height;
    Texture2D texture; // Add texture variable
} Player;



void UnloadPlayer(void); 

//----------------------------------
// Global Player
//----------------------------------

extern Player player;

//----------------------------------
// Player Functions
//----------------------------------

void InitPlayer(void);

void UpdatePlayer(void);

void DrawPlayer(void);

Rectangle GetPlayerRectangle(void);

#endif