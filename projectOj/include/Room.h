#pragma once

#include <vector>
#include <string>

class Player;

class Room {
public:
    enum RoomStatus {
        WAITING,      // Waiting for players to join
        STARTING,     // Match is about to start
        IN_MATCH,     // Match is in progress
        ENDED         // Match has ended
    };

    int roomId;
    std::string roomName;
    int maxPlayers;
    RoomStatus status;
    std::vector<Player*> players;

    Room(int id, const std::string& name, int maxPlayers);
    ~Room();

    bool addPlayer(Player* player);
    bool removePlayer(Player* player);
    bool canJoin() const;
    bool isFull() const;
    int getPlayerCount() const;
    void setStatus(RoomStatus newStatus);
};

