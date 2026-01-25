#pragma once

#include <string>
#include <vector>
#include <map>

class Player;
class Map;
class Room;
class Match;
class Bullet;

class Game {
public:
    static int width;
    static int height;

    enum MenuState {
        NONE,
        ROOM_LIST,      // Displaying list of available rooms
        IN_ROOM,
        PLAYING,
        MATCH_ENDED     // Match finished, showing winner
    };

    static MenuState menuState;
    static Player* currentPlayer;
    static Map* gameMap;
    static Room* currentRoom;
    static Match* currentMatch;
    static std::vector<Room*> rooms;
    static std::vector<Bullet*> bullets;  // All active bullets
    static std::vector<Player*> allPlayers;  // All players in the match
    
    // Mouse position for crosshair (screen coordinates)
    static float mouseX;
    static float mouseY;
    
    // Camera offset (for camera following player)
    static float cameraX;
    static float cameraY;

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
    static void mouseClick(int button, int state, int x, int y);
    static void drawText(float x, float y, const std::string& text);
    static void drawCrosshair(float x, float y);
    static void updateBullets();
    static void cleanupBullets();
    static void checkBulletCollisions();
    static void checkWinCondition();
    
    static Room* createRoom(const std::string& roomName, int maxPlayers);
    static bool joinRoom(int roomId);
    static void startMatch();
    static void requestRoomList();
};
