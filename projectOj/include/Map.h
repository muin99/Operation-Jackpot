#pragma once

#include <vector>
#include <GL/freeglut.h>
#include <cmath>

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
    
    // Create from position and size
    static Rect fromPositionSize(float x, float y, float width, float height) {
        return Rect(x, x + width, y + height, y);
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

class MapPolygon {
public:
    std::vector<float> x;  // X coordinates
    std::vector<float> y;   // Y coordinates
    
    MapPolygon() {}
    
    // Add a vertex
    void addVertex(float x, float y) {
        this->x.push_back(x);
        this->y.push_back(y);
    }
    
    void draw(float red = 0.5f, float green = 0.3f, float blue = 0.1f) const {
        if (x.size() < 3) return;  // Need at least 3 vertices
        
        glColor3f(red, green, blue);
        glBegin(GL_POLYGON);
        for (size_t i = 0; i < x.size(); i++) {
            glVertex2f(x[i], y[i]);
        }
        glEnd();
    }
    
    // Check if a circle (point with radius) collides with this polygon
    bool checkCircleCollision(float px, float py, float radius) const {
        if (x.size() < 3) return false;
        
        // Check if point is inside polygon
        if (pointInPolygon(px, py)) {
            return true;
        }
        
        // Check distance to edges
        float minDist = 999999.0f;
        for (size_t i = 0; i < x.size(); i++) {
            size_t next = (i + 1) % x.size();
            float dist = pointToLineDistance(px, py, x[i], y[i], x[next], y[next]);
            if (dist < minDist) {
                minDist = dist;
            }
        }
        
        return minDist < radius;
    }
    
private:
    // Point-in-polygon test
    bool pointInPolygon(float px, float py) const {
        bool inside = false;
        size_t j = x.size() - 1;
        
        for (size_t i = 0; i < x.size(); i++) {
            if (((y[i] > py) != (y[j] > py)) &&
                (px < (x[j] - x[i]) * (py - y[i]) / (y[j] - y[i]) + x[i])) {
                inside = !inside;
            }
            j = i;
        }
        return inside;
    }
    
    // Distance from point to line segment
    float pointToLineDistance(float px, float py, float x1, float y1, float x2, float y2) const {
        float A = px - x1;
        float B = py - y1;
        float C = x2 - x1;
        float D = y2 - y1;
        
        float dot = A * C + B * D;
        float lenSq = C * C + D * D;
        float param = (lenSq != 0) ? dot / lenSq : -1;
        
        float xx, yy;
        if (param < 0) {
            xx = x1;
            yy = y1;
        } else if (param > 1) {
            xx = x2;
            yy = y2;
        } else {
            xx = x1 + param * C;
            yy = y1 + param * D;
        }
        
        float dx = px - xx;
        float dy = py - yy;
        return sqrt(dx * dx + dy * dy);
    }
};

class Map {
public:
    float width;
    float height;
    std::vector<Rect> rectObstacles;
    std::vector<MapPolygon> polygonObstacles;
    
    Map(float w, float h);
    void render();
    void initializeObstacles();
    
    // Simple collision detection
    bool checkCollision(float x, float y, float radius) const;
    bool checkCollision(const Rect& rect) const;
    bool isValidSpawnPosition(float x, float y, float radius) const;
    
    // Helper to add obstacles
    void addObstacle(float left, float right, float top, float bottom);
    void addObstacle(const Rect& rect);
    void addPolygon(const MapPolygon& polygon);
    
    // Easy polygon creation helpers
    void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
    void addQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
};

