#include <bits/stdc++.h>
#include <GL/freeglut.h>

class Game {
public:
    int width, height;

    Game(int w, int h, int argc, char** argv) {
        width = w;
        height = h;
        runOpenGl(argc, argv);
        display();
    }

    void runOpenGl(int argc, char** argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(width, height);
        glutCreateWindow("OpenGL 64bit");

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glutDisplayFunc(display);

        glutMainLoop();
    }

    static void display() {
        glClear(GL_COLOR_BUFFER_BIT);
        glutSwapBuffers();
    }
};
