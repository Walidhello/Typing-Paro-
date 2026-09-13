#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct
{
    Vector2 position;
    int width;
    int height;
    Texture2D texture; 
} Player;

extern Player player;

void InitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);
Rectangle GetPlayerRectangle(void);
void UnloadPlayer(void); 

#endif
