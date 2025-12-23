#include "Player.h"
#include <GL/freeglut.h>
#include <cmath>

Player::Player() {
    id = -1;
    x = 0.0f;
    y = 0.0f;
    speed = 2.0f;  // Speed per update (updates ~60 times per second)
    size = 20.0f;
    angle = 0.0f;
    // Initialize key states
    for (int i = 0; i < 256; i++) {
        keys[i] = false;
    }
    keyUp = false;
    keyDown = false;
    keyLeft = false;
    keyRight = false;
}

Player::Player(int id, float x, float y) {
    this->id = id;
    this->x = x;
    this->y = y;
    speed = 2.0f;  // Speed per update (updates ~60 times per second)
    size = 20.0f;
    angle = 0.0f;
    // Initialize key states
    for (int i = 0; i < 256; i++) {
        keys[i] = false;
    }
    keyUp = false;
    keyDown = false;
    keyLeft = false;
    keyRight = false;
}

void Player::moveUp() {
    y += speed;
}

void Player::moveDown() {
    y -= speed;
}

void Player::moveLeft() {
    x -= speed;
}

void Player::moveRight() {
    x += speed;
}

void Player::moveUpRight() {
    // Diagonal movement: normalize speed for 45-degree angle
    float diagonalSpeed = speed * 0.707f; // sqrt(2)/2 ≈ 0.707
    x += diagonalSpeed;
    y += diagonalSpeed;
}

void Player::moveUpLeft() {
    float diagonalSpeed = speed * 0.707f;
    x -= diagonalSpeed;
    y += diagonalSpeed;
}

void Player::moveDownLeft() {
    float diagonalSpeed = speed * 0.707f;
    x -= diagonalSpeed;
    y -= diagonalSpeed;
}

void Player::moveDownRight() {
    float diagonalSpeed = speed * 0.707f;
    x += diagonalSpeed;
    y -= diagonalSpeed;
}

void Player::handleKey(unsigned char key, bool pressed) {
    keys[key] = pressed;
}

void Player::handleSpecialKey(int key, bool pressed) {
    switch (key) {
        case GLUT_KEY_UP:
            keyUp = pressed;
            break;
        case GLUT_KEY_DOWN:
            keyDown = pressed;
            break;
        case GLUT_KEY_LEFT:
            keyLeft = pressed;
            break;
        case GLUT_KEY_RIGHT:
            keyRight = pressed;
            break;
    }
}

void Player::updateMovement() {
    // Check WASD keys first
    bool w = keys['w'] || keys['W'];
    bool s = keys['s'] || keys['S'];
    bool a = keys['a'] || keys['A'];
    bool d = keys['d'] || keys['D'];
    
    // Check arrow keys
    bool up = keyUp;
    bool down = keyDown;
    bool left = keyLeft;
    bool right = keyRight;
    
    // Use arrow keys if pressed, otherwise use WASD
    bool shouldMoveUp = up || w;
    bool shouldMoveDown = down || s;
    bool shouldMoveLeft = left || a;
    bool shouldMoveRight = right || d;
    
    // Check for diagonal movements first
    if (shouldMoveUp && shouldMoveRight) {
        moveUpRight();  // WD or Up+Right
    }
    else if (shouldMoveUp && shouldMoveLeft) {
        moveUpLeft();   // WA or Up+Left
    }
    else if (shouldMoveDown && shouldMoveLeft) {
        moveDownLeft(); // SA or Down+Left
    }
    else if (shouldMoveDown && shouldMoveRight) {
        moveDownRight(); // SD or Down+Right
    }
    // Single direction movements
    else if (shouldMoveUp) {
        moveUp();
    }
    else if (shouldMoveDown) {
        moveDown();
    }
    else if (shouldMoveLeft) {
        moveLeft();
    }
    else if (shouldMoveRight) {
        moveRight();
    }
}

void Player::updateAim(float mouseX, float mouseY) {
    float dx = mouseX - x;
    float dy = mouseY - y;
    angle = atan2(dy, dx);
}

void Player::render() {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(angle * 180.0f / 3.14159f, 0.0f, 0.0f, 1.0f);
    
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, size / 2);
    glVertex2f(-size / 2, -size / 2);
    glVertex2f(size / 2, -size / 2);
    glEnd();
    
    glPopMatrix();
}
