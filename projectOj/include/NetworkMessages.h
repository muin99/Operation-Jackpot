#pragma once

#include <cstdint>
#include <string>
#include <cstring>

// Message structures for network communication

struct ClientConnectMessage {
    char playerName[32];
};

struct ServerAcceptMessage {
    int playerId;
    int assignedId;
};

struct CreateRoomRequestMessage {
    char roomName[64];
    int maxPlayers;
};

struct CreateRoomResponseMessage {
    bool success;
    int roomId;
    char roomName[64];
    int maxPlayers;
};

struct JoinRoomRequestMessage {
    int roomId;
};

struct JoinRoomResponseMessage {
    bool success;
    int roomId;
    char roomName[64];
    int maxPlayers;
    int playerCount;
};

struct RoomInfoMessage {
    int roomId;
    char roomName[64];
    int playerCount;
    int maxPlayers;
    int status;  // Room::RoomStatus
};

struct RoomListResponseMessage {
    int roomCount;
    // Followed by RoomInfoMessage array
};

struct StartMatchRequestMessage {
    int roomId;
};

struct StartMatchResponseMessage {
    bool success;
    int matchId;
};

struct PlayerInputMessage {
    uint8_t keys;  // Bit flags: bit 0=W, bit 1=S, bit 2=A, bit 3=D, bit 4=shoot
    float mouseX;
    float mouseY;
};

struct PlayerStateMessage {
    int playerId;
    float x;
    float y;
    float angle;
};

struct BulletSpawnMessage {
    int bulletId;
    float x;
    float y;
    float vx;
    float vy;
    int ownerId;
};

struct BulletUpdateMessage {
    int bulletId;
    float x;
    float y;
};

struct BulletRemoveMessage {
    int bulletId;
};

struct PingMessage {
    uint32_t timestamp;
};

struct PongMessage {
    uint32_t timestamp;
};

