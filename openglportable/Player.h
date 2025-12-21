#pragma once
#include <GL/freeglut.h>

class Player {
public:
    int id;
    float x, y;
    float speed;
    float size;
    float angle;

    Player();
    Player(int id, float x, float y);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void handleKey(unsigned char key);
    void handleSpecialKey(int key);
    void updateAim(float mouseX, float mouseY);
    void render();
};
