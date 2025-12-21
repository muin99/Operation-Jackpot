#include "Game.h"
#include "Player.h"
#include <GL/freeglut.h>
#include <iostream>

int Game::width = 0;
int Game::height = 0;
Game::MenuState Game::menuState = Game::NONE;
Player* Game::currentPlayer = nullptr;

Game::Game(int w, int h, int argc, char** argv) {
    width = w;
    height = h;
    runOpenGl(argc, argv);
}

void Game::runOpenGl(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("2D Battle Royale");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyPressed);
    glutSpecialFunc(specialKeyPressed);
    glutPassiveMotionFunc(mouseMotion);

    glutMainLoop();
}

void Game::display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);

    if (menuState == NONE) {
        drawText(300, 350, "J - Join Room");
        drawText(300, 300, "C - Create Room");
    }
    else if (menuState == JOIN_ROOM) {
        drawText(280, 325, "Joining Room...");
    }
    else if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->render();
        }
    }

    glutSwapBuffers();
}

void Game::keyPressed(unsigned char key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->handleKey(key);
        }
        if (key == 27) {
            menuState = NONE;
        }
    }
    else {
        if (key == 'j' || key == 'J') {
            menuState = JOIN_ROOM;
            std::cout << "Join Room selected\n";
        }
        else if (key == 'c' || key == 'C') {
            if (currentPlayer == nullptr) {
                currentPlayer = new Player(1, width/2, height/2);
            }
            menuState = PLAYING;
            std::cout << "Create Room selected\n";
        }
        else if (key == 27) {
            menuState = NONE;
            std::cout << "Back to menu\n";
        }
    }

    glutPostRedisplay();
}

void Game::specialKeyPressed(int key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->handleSpecialKey(key);
        }
        glutPostRedisplay();
    }
}

void Game::mouseMotion(int x, int y) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            float mouseX = x;
            float mouseY = height - y;
            currentPlayer->updateAim(mouseX, mouseY);
            glutPostRedisplay();
        }
    }
}

void Game::drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}
