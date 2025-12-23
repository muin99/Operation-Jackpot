#include "Map.h"
#include <GL/freeglut.h>
#include <cmath>

Map::Map(float w, float h) {
    width = w;
    height = h;
}

void Map::render() {
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    glColor3f(0.3f, 0.3f, 0.3f);
    float borderSize = 20.0f;
    
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, borderSize);
    glVertex2f(0, borderSize);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(0, height - borderSize);
    glVertex2f(width, height - borderSize);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(borderSize, 0);
    glVertex2f(borderSize, height);
    glVertex2f(0, height);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(width - borderSize, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(width - borderSize, height);
    glEnd();
    
    glColor3f(0.4f, 0.4f, 0.4f);
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float circleRadius = 100.0f;
    int segments = 32;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(centerX, centerY);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        glVertex2f(centerX + circleRadius * cos(angle), centerY + circleRadius * sin(angle));
    }
    glEnd();
}

