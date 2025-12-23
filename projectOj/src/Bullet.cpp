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
    
    // Calculate the angle of travel from velocity
    float travelAngle = atan2(vy, vx);
    
    // Triangle size
    float triangleSize = size * 1.5f;  // Make triangle slightly larger than the old circle
    
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow color for bullets
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(travelAngle * 180.0f / 3.14159f, 0.0f, 0.0f, 1.0f);
    
    // Draw a sharp triangle pointing forward (in direction of travel)
    glBegin(GL_TRIANGLES);
    // Tip of triangle (pointing forward)
    glVertex2f(triangleSize, 0.0f);
    // Base vertices (back of triangle)
    glVertex2f(-triangleSize / 2.0f, triangleSize / 2.0f);
    glVertex2f(-triangleSize / 2.0f, -triangleSize / 2.0f);
    glEnd();
    
    glPopMatrix();
}

bool Bullet::isOutOfBounds(int screenWidth, int screenHeight) {
    return x < 0 || x > screenWidth || y < 0 || y > screenHeight;
}

