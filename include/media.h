#ifndef MEDIA_H
#define MEDIA_H

#include "raylib.h"
#include <stdbool.h>

// ============================================================================
// MEDIA & ASSET HOOKS
// - Background: Drop a file named "background.png" (or "background.jpg", "bg.png")
//   into the "assets/" folder. The game will automatically detect and render it!
// - Music: Drop a file named "music.mp3" (or "music.ogg", "bgm.mp3", "music.wav")
//   into the "assets/" folder. The game will automatically loop and stream it!
// ============================================================================

void InitMedia(void);
void UpdateMedia(void);
void DrawCustomBackground(int screenWidth, int screenHeight);
void DrawCosmicStarfield(int screenWidth, int screenHeight, float speedMultiplier);
void UnloadMedia(void);

bool HasCustomBackground(void);
bool HasCustomMusic(void);
void ToggleMusicMute(void);
bool IsMusicMuted(void);

#endif
