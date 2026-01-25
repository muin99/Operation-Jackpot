#pragma once
#include <GL/freeglut.h>

class Bullet;

class Player {
public:
    int id;
    float x, y;
    float speed;
    float size;
    float angle;
    bool isAlive;  // Whether player is still alive

    // Key state tracking for diagonal movement
    bool keys[256];
    bool keyUp;
    bool keyDown;
    bool keyLeft;
    bool keyRight;

    Player();
    Player(int id, float x, float y);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void moveUpRight();   // WD - diagonal up-right
    void moveUpLeft();    // WA - diagonal up-left
    void moveDownLeft();  // SA - diagonal down-left
    void moveDownRight(); // SD - diagonal down-right
    
    void handleKey(unsigned char key, bool pressed);
    void handleSpecialKey(int key, bool pressed);
    void updateMovement();  // Process movement based on key states
    void updateMovementWithCollision(bool (*checkCollision)(float x, float y, float radius));  // Movement with collision check
    void updateAim(float mouseX, float mouseY);
    void shoot(float mouseX, float mouseY);  // Shoot a bullet towards mouse position
    void render();
    void eliminate();  // Mark player as eliminated
    
    // Get bullet spawn position (slightly in front of player)
    void getBulletSpawnPosition(float& outX, float& outY);
    
    // Movement helpers that check collision
    bool tryMoveTo(float newX, float newY, bool (*checkCollision)(float x, float y, float radius));
};
