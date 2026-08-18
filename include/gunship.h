#ifndef GUNSHIP_H
#define GUNSHIP_H

#include "raylib.h"

typedef struct
{
    Vector2 position;
    float width;
    float height;
} Gunship;

extern Gunship gunship;

void InitGunship(void);
void UpdateGunship(void);
void DrawGunship(void);

#endif