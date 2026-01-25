#pragma once

#include <vector>
#include <GL/freeglut.h>

class Rect {
public:
    float left;
    float right;
    float top;
    float bottom;
    
    Rect() : left(0), right(0), top(0), bottom(0) {}
    
    Rect(float left, float right, float top, float bottom) {
        this->left = left;
        this->right = right;
        this->top = top;
        this->bottom = bottom;
    }
    
    void draw(float red = 0.5f, float green = 0.3f, float blue = 0.1f) const {
        glColor3f(red, green, blue);
        glBegin(GL_QUADS);
            glVertex2f(left, top);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
        glEnd();
    }
    
    bool checkCollision(const Rect& other) const {
        if (other.right < left) return false;
        if (other.left > right) return false;
        if (other.top < bottom) return false;
        if (other.bottom > top) return false;
        return true;
    }
    
    // Check if a circle (point with radius) collides with this rectangle
    bool checkCircleCollision(float x, float y, float radius) const {
        // Find closest point on rectangle to circle center
        float closestX = (x < left) ? left : ((x > right) ? right : x);
        float closestY = (y < bottom) ? bottom : ((y > top) ? top : y);
        
        float dx = x - closestX;
        float dy = y - closestY;
        float distanceSquared = dx * dx + dy * dy;
        
        return distanceSquared < radius * radius;
    }
};

class Map {
public:
    float width;
    float height;
    std::vector<Rect> collisionRects;  // For collision detection only
    
    Map(float w, float h);
    void render();
    void initializeMap();
    
    // Collision detection
    bool checkCollision(float x, float y, float radius) const;
    bool isValidSpawnPosition(float x, float y, float radius) const;
};
