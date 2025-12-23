#pragma once

#include <string>
#include <vector>

class Player;
class Map;
class Room;
class Match;

class Game {
public:
    static int width;
    static int height;

    enum MenuState {
        NONE,
        JOIN_ROOM,
        CREATE_ROOM,
        IN_ROOM,
        PLAYING
    };

    static MenuState menuState;
    static Player* currentPlayer;
    static Map* gameMap;
    static Room* currentRoom;
    static Match* currentMatch;
    static std::vector<Room*> rooms;

    Game(int w, int h, int argc, char** argv);

    static void runOpenGl(int argc, char** argv);
    static void display();
    static void idle();
    static void timer(int value);
    static void keyPressed(unsigned char key, int x, int y);
    static void keyUp(unsigned char key, int x, int y);
    static void specialKeyPressed(int key, int x, int y);
    static void specialKeyUp(int key, int x, int y);
    static void mouseMotion(int x, int y);
    static void drawText(float x, float y, const std::string& text);
    
    static Room* createRoom(const std::string& roomName, int maxPlayers);
    static bool joinRoom(int roomId);
    static void startMatch();
};
