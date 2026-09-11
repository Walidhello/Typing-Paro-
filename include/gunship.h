#ifndef GUNSHIP_H
#define GUNSHIP_H

#include "raylib.h"

typedef struct
{
    Vector2 position;
    float width;
    float height;

    float rotation;
    float targetRotation;

    RenderTexture2D texture;
} Gunship;

extern Gunship gunship;

void InitGunship(void);
void UnloadGunship(void);
void UpdateGunship(void);
void DrawGunshipDesign(void);
void DrawGunship(void);

#endif