#ifndef MEDIA_H
#define MEDIA_H

#include "raylib.h"
#include <stdbool.h>

void InitMedia(void);
void UpdateMedia(void);
void DrawCustomBackground(int screenWidth, int screenHeight);
void DrawCosmicStarfield(int screenWidth, int screenHeight, float speedMultiplier);
void UnloadMedia(void);

bool HasCustomBackground(void);
bool HasCustomMusic(void);
void ToggleMusicMute(void);
bool IsMusicMuted(void);

void PlayLaserSound(bool isKillShot);
void PlayMisfireSound(void);

#endif
