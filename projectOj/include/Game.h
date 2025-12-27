#pragma once

#include <string>
#include <vector>
#include <map>

class Player;
class Map;
class Room;
class Match;
class Bullet;
class NetworkManager;

// Forward declare network message types
struct RoomInfoMessage;

class Game {
public:
    static int width;
    static int height;

    enum MenuState {
        NONE,
        ROOM_LIST,      // Displaying list of available rooms
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
    static std::vector<Bullet*> bullets;  // All active bullets
    static std::map<int, Player*> remotePlayers;  // Remote players (playerId -> Player)
    static std::map<int, Bullet*> networkBullets;  // Network-synchronized bullets (bulletId -> Bullet)
    static std::vector<RoomInfoMessage> availableRooms;  // Available rooms from server
    
    // Networking
    static NetworkManager* networkManager;
    static bool isServer;
    static bool isClient;
    static std::string serverIP;
    static int serverPort;
    static bool autoStartServerFlag;
    static bool autoStartClientFlag;
    static std::string autoStartClientIP;
    
    // Mouse position for crosshair (screen coordinates)
    static float mouseX;
    static float mouseY;
    
    // Camera offset (for camera following player)
    static float cameraX;
    static float cameraY;
    
    // Input state for networking
    static bool lastShootState;
    static float lastInputSendTime;

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
    
    static Room* createRoom(const std::string& roomName, int maxPlayers);
    static bool joinRoom(int roomId);
    static void startMatch();
    
    // Network functions
    static void initializeNetwork(bool asServer, const std::string& serverIP = "", int port = 7777);
    static void shutdownNetwork();
    static void updateNetwork();
    static void sendPlayerInput();
    static void handleNetworkPlayerState(int playerId, float x, float y, float angle);
    static void handleNetworkBulletSpawn(int bulletId, float x, float y, float vx, float vy, int ownerId);
    static void handleNetworkBulletRemove(int bulletId);
    static void handleNetworkRoomUpdate(int roomId, const std::string& name, int playerCount, int maxPlayers);
    static void handleNetworkRoomListResponse(const std::vector<RoomInfoMessage>& roomList);
    static void handleNetworkJoinRoomResponse(bool success, int roomId);
    static void handleNetworkCreateRoomResponse(bool success, int roomId);
    static void handleNetworkMatchStart();
    
    // Room list functions
    static void requestRoomList();
    static void displayRoomList();
    
    // Server-specific functions
    static void updateServerGameState();
    static void broadcastPlayerStates();
    static void broadcastBulletStates();
};
