#ifndef SHOOTING_H
#define SHOOTING_H

#include "raylib.h"
#include "enemy.h"
#include <stdbool.h>

#define MAX_PROJECTILES 64
#define MAX_PARTICLES 300
#define MAX_MUZZLE_FLASHES 16
#define MAX_SHOCKWAVES 12
#define MAX_FLOAT_TEXTS 10

// Laser Projectile
typedef struct {
    bool active;
    Vector2 position;
    Vector2 velocity;
    Vector2 target;
    int targetEnemyIndex;
    bool isKillShot;
    EnemyType enemyType;
    Color coreColor;
    Color glowColor;
    float speed;
    float length;
    float width;
    Vector2 trail[6];
    int trailCount;
} LaserProjectile;

// Muzzle Flash
typedef struct {
    bool active;
    Vector2 position;
    float timer;
    float maxTimer;
    float radius;
    Color color;
} MuzzleFlash;

// Particle Effect (Sparks, Smoke, Fire, Exhaust)
typedef struct {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float size;
    float timer;
    float maxTimer;
    Color startColor;
    Color endColor;
    float drag;
    bool isSmoke;
} VFXParticle;

// Expanding Shockwave Ring
typedef struct {
    bool active;
    Vector2 position;
    float radius;
    float maxRadius;
    float speed;
    float thickness;
    Color color;
} Shockwave;

// Floating Combat Text
typedef struct {
    bool active;
    Vector2 position;
    char text[32];
    float timer;
    float maxTimer;
    Color color;
    int fontSize;
} FloatText;

// Function Declarations
void InitShooting(void);
void UpdateShooting(void);
void DrawShootingProjectiles(void);
void DrawShootingEffects(void);

void FireLaser(int targetIndex, bool isKillShot);
void TriggerMisfire(void);
void CreateExplosion(Vector2 pos, EnemyType type, const char* word);
void CreateHitSparks(Vector2 pos, Color color, int count);
void AddMuzzleFlash(Vector2 pos, Color color);
void AddExhaustParticle(Vector2 pos, Vector2 vel, Color color);

Vector2 GetScreenShakeOffset(void);

#endif
