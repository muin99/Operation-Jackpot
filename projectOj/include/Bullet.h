#pragma once
#include <GL/freeglut.h>

class Bullet {
public:
    float x, y;           // Position
    float vx, vy;         // Velocity
    float speed;          // Bullet speed
    float size;           // Bullet size
    bool active;          // Whether bullet is still active
    int ownerId;          // ID of player who shot this bullet
    float lifetime;        // Time bullet has been alive
    float maxLifetime;    // Maximum lifetime before bullet despawns

    Bullet();
    Bullet(float startX, float startY, float angle, int ownerId, float bulletSpeed = 10.0f);
    
    void update();
    void render();
    bool isOutOfBounds(int screenWidth, int screenHeight);
};

