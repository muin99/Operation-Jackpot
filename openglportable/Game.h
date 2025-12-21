#pragma once

#include <string>

class Player;

class Game {
public:
    static int width;
    static int height;

    enum MenuState {
        NONE,
        JOIN_ROOM,
        CREATE_ROOM,
        PLAYING
    };

    static MenuState menuState;
    static Player* currentPlayer;

    Game(int w, int h, int argc, char** argv);

    static void runOpenGl(int argc, char** argv);
    static void display();
    static void keyPressed(unsigned char key, int x, int y);
    static void specialKeyPressed(int key, int x, int y);
    static void mouseMotion(int x, int y);
    static void drawText(float x, float y, const std::string& text);
};
