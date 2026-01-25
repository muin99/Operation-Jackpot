#include "Map.h"
#include <GL/freeglut.h>
#include <cmath>

Map::Map(float w, float h) {
    width = w;
    height = h;
    initializeMap();
}

void Map::initializeMap() {
    collisionRects.clear();
    
    // Add collision rectangles for borders
    float borderSize = 20.0f;
    collisionRects.push_back(Rect(0, borderSize, height, 0));  // Left wall
    collisionRects.push_back(Rect(width - borderSize, width, height, 0));  // Right wall
    collisionRects.push_back(Rect(0, width, borderSize, 0));  // Bottom wall
    collisionRects.push_back(Rect(0, width, height, height - borderSize));  // Top wall
    
    // Add collision rectangles for obstacles
    // IMPORTANT: These must match the obstacles you draw in render()!
    // For each obstacle in render(), add a corresponding collision rect here
    
    // Obstacle 1: Center at (150, 125), Size 100x50
    collisionRects.push_back(Rect(100, 200, 150, 100));
    
    // Obstacle 2: Center at (350, 175), Size 100x50, Rotated 45 degrees
    // For rotated rectangles, use a bounding box that covers the rotation
    float rotSize = 100.0f * 1.414f;  // Diagonal of 100x50 rectangle
    collisionRects.push_back(Rect(350 - rotSize/2, 350 + rotSize/2, 175 + rotSize/2, 175 - rotSize/2));
    
    // Obstacle 3: Center at (550, 225), Size 100x50
    collisionRects.push_back(Rect(500, 600, 250, 200));
    
    // Obstacle 4: Center at (250, 375), Size 80x80, Rotated 30 degrees
    float rotSize2 = 80.0f * 1.414f;
    collisionRects.push_back(Rect(250 - rotSize2/2, 250 + rotSize2/2, 375 + rotSize2/2, 375 - rotSize2/2));
    
    // Obstacle 5: Center at (500, 425), Size 120x40
    collisionRects.push_back(Rect(440, 560, 445, 405));
}

// Helper function to draw a unit square (centered at origin, size 1x1)
void drawUnitSquare() {
    glBegin(GL_QUADS);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f( 0.5f, -0.5f);
        glVertex2f( 0.5f,  0.5f);
        glVertex2f(-0.5f,  0.5f);
    glEnd();
}

void Map::render() {
    // Draw background
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    // ============================================
    // DRAW MAP OBSTACLES HERE
    // Use glPushMatrix, glTranslatef, glRotatef, glScalef, glPopMatrix
    // ============================================
    
    // Border walls
    float borderSize = 20.0f;
    
    // Left wall
    glPushMatrix();
        glTranslatef(borderSize / 2.0f, height / 2.0f, 0.0f);
        glScalef(borderSize, height, 1.0f);
        glColor3f(0.5f, 0.3f, 0.1f);  // Brown
        drawUnitSquare();
    glPopMatrix();
    
    // Right wall
    glPushMatrix();
        glTranslatef(width - borderSize / 2.0f, height / 2.0f, 0.0f);
        glScalef(borderSize, height, 1.0f);
        glColor3f(0.5f, 0.3f, 0.1f);  // Brown
        drawUnitSquare();
    glPopMatrix();
    
    // Bottom wall
    glPushMatrix();
        glTranslatef(width / 2.0f, borderSize / 2.0f, 0.0f);
        glScalef(width, borderSize, 1.0f);
        glColor3f(0.5f, 0.3f, 0.1f);  // Brown
        drawUnitSquare();
    glPopMatrix();
    
    // Top wall
    glPushMatrix();
        glTranslatef(width / 2.0f, height - borderSize / 2.0f, 0.0f);
        glScalef(width, borderSize, 1.0f);
        glColor3f(0.5f, 0.3f, 0.1f);  // Brown
        drawUnitSquare();
    glPopMatrix();
    
    // Example obstacle 1 - Simple rectangle
    glPushMatrix();
        glTranslatef(150.0f, 125.0f, 0.0f);  // Center at (150, 125)
        glScalef(100.0f, 50.0f, 1.0f);       // Size: 100x50
        glColor3f(0.9f, 0.0f, 0.0f);        // Red
        drawUnitSquare();
    glPopMatrix();
    
    // Example obstacle 2 - Rotated rectangle
    glPushMatrix();
        glTranslatef(350.0f, 175.0f, 0.0f);  // Center at (350, 175)
        glRotatef(45.0f, 0.0f, 0.0f, 1.0f);   // Rotate 45 degrees
        glScalef(100.0f, 50.0f, 1.0f);        // Size: 100x50
        glColor3f(0.0f, 0.9f, 0.0f);          // Green
        drawUnitSquare();
    glPopMatrix();
    
    // Example obstacle 3 - Scaled rectangle
    glPushMatrix();
        glTranslatef(550.0f, 225.0f, 0.0f);   // Center at (550, 225)
        glScalef(100.0f, 50.0f, 1.0f);        // Size: 100x50
        glColor3f(0.0f, 0.0f, 0.9f);          // Blue
        drawUnitSquare();
    glPopMatrix();
    
    // Example obstacle 4 - Combined transformations
    glPushMatrix();
        glTranslatef(250.0f, 375.0f, 0.0f);   // Translate first
        glRotatef(30.0f, 0.0f, 0.0f, 1.0f);   // Then rotate
        glScalef(80.0f, 80.0f, 1.0f);          // Then scale
        glColor3f(0.9f, 0.9f, 0.0f);         // Yellow
        drawUnitSquare();
    glPopMatrix();
    
    // Example obstacle 5 - Different scale on X and Y
    glPushMatrix();
        glTranslatef(500.0f, 425.0f, 0.0f);
        glScalef(120.0f, 40.0f, 1.0f);        // Wide rectangle
        glColor3f(0.9f, 0.0f, 0.9f);          // Magenta
        drawUnitSquare();
    glPopMatrix();
    
    // ============================================
    // ADD MORE OBSTACLES HERE USING TRANSFORMATIONS
    // ============================================
}

bool Map::checkCollision(float x, float y, float radius) const {
    for (const Rect& rect : collisionRects) {
        if (rect.checkCircleCollision(x, y, radius)) {
            return true;
        }
    }
    return false;
}

bool Map::isValidSpawnPosition(float x, float y, float radius) const {
    // Check bounds
    float margin = radius + 10.0f;
    if (x < margin || x > width - margin || y < margin || y > height - margin) {
        return false;
    }
    
    // Check collision
    return !checkCollision(x, y, radius);
}
