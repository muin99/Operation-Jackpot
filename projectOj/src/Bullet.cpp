#include "Bullet.h"
#include <cmath>

Bullet::Bullet() {
    x = 0.0f;
    y = 0.0f;
    vx = 0.0f;
    vy = 0.0f;
    speed = 10.0f;
    size = 4.0f;
    active = false;
    ownerId = -1;
    lifetime = 0.0f;
    maxLifetime = 3.0f;  // Bullets despawn after 3 seconds
}

Bullet::Bullet(float startX, float startY, float angle, int ownerId, float bulletSpeed) {
    this->x = startX;
    this->y = startY;
    this->ownerId = ownerId;
    this->speed = bulletSpeed;
    this->size = 4.0f;
    this->active = true;
    this->lifetime = 0.0f;
    this->maxLifetime = 3.0f;
    
    // Calculate velocity based on angle
    // Note: angle is already adjusted for the arrow pointing direction
    // We need to add π/2 back to get the correct direction
    float adjustedAngle = angle + 3.14159f / 2.0f;
    vx = cos(adjustedAngle) * speed;
    vy = sin(adjustedAngle) * speed;
}

void Bullet::update() {
    if (!active) return;
    
    // Update position
    x += vx;
    y += vy;
    
    // Update lifetime
    lifetime += 0.016f;  // Assuming ~60 FPS (16ms per frame)
    
    // Deactivate if lifetime exceeded
    if (lifetime >= maxLifetime) {
        active = false;
    }
}

void Bullet::render() {
    if (!active) return;
    
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow color for bullets
    glPointSize(size);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
    
    // Also draw a small circle for better visibility
    glBegin(GL_TRIANGLE_FAN);
    int segments = 8;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        glVertex2f(x + cos(angle) * size / 2.0f, y + sin(angle) * size / 2.0f);
    }
    glEnd();
}

bool Bullet::isOutOfBounds(int screenWidth, int screenHeight) {
    return x < 0 || x > screenWidth || y < 0 || y > screenHeight;
}

