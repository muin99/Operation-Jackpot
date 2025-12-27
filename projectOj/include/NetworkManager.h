#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstdint>
#include "NetworkMessages.h"

// Forward declare ENet types to avoid requiring ENet headers in header file
struct _ENetHost;
struct _ENetPeer;
struct _ENetPacket;
typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
typedef struct _ENetPacket ENetPacket;

// Network message types
enum class MessageType : uint8_t {
    // Connection
    CLIENT_CONNECT = 0,
    CLIENT_DISCONNECT = 1,
    SERVER_ACCEPT = 2,
    SERVER_REJECT = 3,
    
    // Room management
    CREATE_ROOM_REQUEST = 10,
    CREATE_ROOM_RESPONSE = 11,
    JOIN_ROOM_REQUEST = 12,
    JOIN_ROOM_RESPONSE = 13,
    ROOM_LIST_REQUEST = 14,
    ROOM_LIST_RESPONSE = 15,
    ROOM_UPDATE = 16,
    
    // Match management
    START_MATCH_REQUEST = 20,
    START_MATCH_RESPONSE = 21,
    MATCH_STATE_UPDATE = 22,
    
    // Game state
    PLAYER_INPUT = 30,          // Client sends input (movement keys, mouse)
    PLAYER_STATE = 31,          // Server sends player positions/angles
    BULLET_SPAWN = 32,          // Server sends new bullet
    BULLET_UPDATE = 33,         // Server sends bullet positions
    BULLET_REMOVE = 34,         // Server sends bullet removal
    
    // Ping/Pong for latency measurement
    PING = 40,
    PONG = 41
};

// Network mode
enum class NetworkMode {
    NONE,       // Single player / offline
    SERVER,     // Running as server
    CLIENT      // Running as client
};

// Forward declarations
class Player;
class Room;
class Match;
class Bullet;

class NetworkManager {
public:
    static NetworkManager* instance;
    
    NetworkMode mode;
    ENetHost* host;
    ENetPeer* serverPeer;  // For clients: connection to server
    std::map<ENetPeer*, int> peerToPlayerId;  // Map peer to player ID
    std::map<int, ENetPeer*> playerIdToPeer;   // Map player ID to peer
    int nextPlayerId;
    int localPlayerId;     // Our player ID (for clients)
    
    bool isConnected;
    float lastPingTime;
    float pingInterval;
    
    // Callbacks
    std::function<void(int playerId, float x, float y, float angle)> onPlayerStateUpdate;
    std::function<void(int bulletId, float x, float y, float vx, float vy, int ownerId)> onBulletSpawn;
    std::function<void(int bulletId, float x, float y)> onBulletUpdate;
    std::function<void(int bulletId)> onBulletRemove;
    std::function<void(int roomId, const std::string& name, int playerCount, int maxPlayers)> onRoomUpdate;
    std::function<void(const std::vector<RoomInfoMessage>& roomList)> onRoomListResponse;
    std::function<void(bool success, int roomId)> onJoinRoomResponse;
    std::function<void(bool success, int roomId)> onCreateRoomResponse;
    std::function<void()> onMatchStart;
    
    // Server callbacks
    std::function<void(int playerId, bool w, bool s, bool a, bool d, float mouseX, float mouseY, bool shoot)> onPlayerInput;
    
    NetworkManager();
    ~NetworkManager();
    
    // Initialize networking
    bool initialize();
    void shutdown();
    
    // Server functions
    bool startServer(int port = 7777);
    void stopServer();
    void updateServer();  // Call this in game loop
    void broadcastToAll(MessageType type, const void* data, size_t dataSize, bool reliable = true);
    void sendToPeer(ENetPeer* peer, MessageType type, const void* data, size_t dataSize, bool reliable = true);
    
    // Client functions
    bool connectToServer(const std::string& hostname, int port = 7777);
    void disconnectFromServer();
    void updateClient();  // Call this in game loop
    void sendToServer(MessageType type, const void* data, size_t dataSize, bool reliable = true);
    
    // Common update function (calls server or client update)
    void update();
    
    // Message sending helpers
    void sendPlayerInput(bool w, bool s, bool a, bool d, float mouseX, float mouseY, bool shoot);
    void sendCreateRoomRequest(const std::string& roomName, int maxPlayers);
    void sendJoinRoomRequest(int roomId);
    void sendRoomListRequest();
    void sendStartMatchRequest();
    
    // Message receiving
    void handleMessage(ENetPeer* peer, ENetPacket* packet);
    
    // Utility
    static std::string getLocalIP();
    static std::vector<std::string> getLocalIPs();
};

