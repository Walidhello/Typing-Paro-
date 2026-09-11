#ifndef GUNSHIP_H
#define GUNSHIP_H

#include "raylib.h"

typedef struct
{
    Vector2 position;
    float width;
    float height;
    float recoilY;
    float cannonGlowLeft;
    float cannonGlowRight;
    float flameBoost;
    float misfireTimer;
} Gunship;

extern Gunship gunship;

void InitGunship(void);
void UpdateGunship(void);
void DrawGunship(void);

#endif
