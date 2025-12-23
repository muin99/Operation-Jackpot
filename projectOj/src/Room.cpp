#include "Room.h"
#include "Player.h"

Room::Room(int id, const std::string& name, int maxPlayers)
    : roomId(id), roomName(name), maxPlayers(maxPlayers), status(WAITING) {
}

Room::~Room() {
    // Cleanup will be handled by Match/Game
}

bool Room::addPlayer(Player* player) {
    if (isFull() || status != WAITING) {
        return false;
    }
    players.push_back(player);
    return true;
}

bool Room::removePlayer(Player* player) {
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (*it == player) {
            players.erase(it);
            return true;
        }
    }
    return false;
}

bool Room::canJoin() const {
    return status == WAITING && !isFull();
}

bool Room::isFull() const {
    return players.size() >= maxPlayers;
}

int Room::getPlayerCount() const {
    return players.size();
}

void Room::setStatus(RoomStatus newStatus) {
    status = newStatus;
}

