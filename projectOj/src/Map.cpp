#include "Map.h"
#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>

Map::Map(float w, float h) {
    width = w;
    height = h;
    initializeObstacles();
}

void Map::initializeObstacles() {
    rectObstacles.clear();
    polygonObstacles.clear();
    
    // Simple border walls
    float borderSize = 20.0f;
    addObstacle(0, borderSize, height, 0);  // Left wall
    addObstacle(width - borderSize, width, height, 0);  // Right wall
    addObstacle(0, width, borderSize, 0);  // Bottom wall
    addObstacle(0, width, height, height - borderSize);  // Top wall
    
    // ============================================
    // MAP DESIGN - Simple approach
    // ============================================
    // Add rectangles: addObstacle(left, right, top, bottom)
    // Add polygons: addTriangle(x1, y1, x2, y2, x3, y3)
    //              or addQuad(x1, y1, x2, y2, x3, y3, x4, y4)
    // ============================================
    
    // Example rectangles
    addObstacle(100, 200, 150, 100);
    addObstacle(300, 400, 200, 150);
    addObstacle(500, 600, 250, 200);
    
    // Example polygons
    addTriangle(200, 300, 250, 250, 300, 300);  // Triangle
    addQuad(400, 400, 500, 400, 500, 500, 400, 500);  // Quad
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
    
    // Draw rectangle obstacles
    for (const Rect& obstacle : rectObstacles) {
        obstacle.draw(0.5f, 0.3f, 0.1f);  // Brown color
    }
    
    // Draw polygon obstacles
    for (const MapPolygon& obstacle : polygonObstacles) {
        obstacle.draw(0.5f, 0.3f, 0.1f);  // Brown color
    }
}

bool Map::checkCollision(float x, float y, float radius) const {
    // Check rectangle obstacles
    for (const Rect& obstacle : rectObstacles) {
        if (obstacle.checkCircleCollision(x, y, radius)) {
            return true;
        }
    }
    
    // Check polygon obstacles
    for (const MapPolygon& obstacle : polygonObstacles) {
        if (obstacle.checkCircleCollision(x, y, radius)) {
            return true;
        }
    }
    
    return false;
}

bool Map::checkCollision(const Rect& rect) const {
    // Check rectangle obstacles
    for (const Rect& obstacle : rectObstacles) {
        if (obstacle.checkCollision(rect)) {
            return true;
        }
    }
    
    // Check polygon obstacles (convert rect to circle check at center)
    float centerX = (rect.left + rect.right) / 2.0f;
    float centerY = (rect.top + rect.bottom) / 2.0f;
    float radius = std::max((rect.right - rect.left) / 2.0f, (rect.top - rect.bottom) / 2.0f);
    
    for (const MapPolygon& obstacle : polygonObstacles) {
        if (obstacle.checkCircleCollision(centerX, centerY, radius)) {
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

void Map::addObstacle(float left, float right, float top, float bottom) {
    rectObstacles.push_back(Rect(left, right, top, bottom));
}

void Map::addObstacle(const Rect& rect) {
    rectObstacles.push_back(rect);
}

void Map::addPolygon(const MapPolygon& polygon) {
    polygonObstacles.push_back(polygon);
}

void Map::addTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    MapPolygon triangle;
    triangle.addVertex(x1, y1);
    triangle.addVertex(x2, y2);
    triangle.addVertex(x3, y3);
    polygonObstacles.push_back(triangle);
}

void Map::addQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    MapPolygon quad;
    quad.addVertex(x1, y1);
    quad.addVertex(x2, y2);
    quad.addVertex(x3, y3);
    quad.addVertex(x4, y4);
    polygonObstacles.push_back(quad);
}
