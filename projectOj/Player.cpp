#include "Player.h"
#include <GL/freeglut.h>
#include <cmath>

Player::Player() {
    id = -1;
    x = 0.0f;
    y = 0.0f;
    speed = 5.0f;
    size = 20.0f;
    angle = 0.0f;
}

Player::Player(int id, float x, float y) {
    this->id = id;
    this->x = x;
    this->y = y;
    speed = 5.0f;
    size = 20.0f;
    angle = 0.0f;
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

void Player::handleKey(unsigned char key) {
    if (key == 'w' || key == 'W') {
        moveUp();
    }
    else if (key == 's' || key == 'S') {
        moveDown();
    }
    else if (key == 'a' || key == 'A') {
        moveLeft();
    }
    else if (key == 'd' || key == 'D') {
        moveRight();
    }
}

void Player::handleSpecialKey(int key) {
    if (key == GLUT_KEY_UP) {
        moveUp();
    }
    else if (key == GLUT_KEY_DOWN) {
        moveDown();
    }
    else if (key == GLUT_KEY_LEFT) {
        moveLeft();
    }
    else if (key == GLUT_KEY_RIGHT) {
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
