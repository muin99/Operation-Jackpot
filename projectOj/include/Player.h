#pragma once
#include <GL/freeglut.h>

class Player {
public:
    int id;
    float x, y;
    float speed;
    float size;
    float angle;

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
    void updateAim(float mouseX, float mouseY);
    void render();
};
