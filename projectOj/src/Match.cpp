#include "Match.h"
#include "Room.h"
#include "Player.h"
#include "Map.h"

Match::Match(int id, Room* room)
    : matchId(id), state(PREPARING), gameMap(nullptr), sourceRoom(room) {
    // Copy players from room
    if (room != nullptr) {
        players = room->players;
    }
}

Match::~Match() {
    // Cleanup handled by Game
}

void Match::start() {
    state = IN_PROGRESS;
    // Initialize game map if not already set
    if (gameMap == nullptr && sourceRoom != nullptr) {
        // Map will be created by Game class before calling start()
        // This is just a placeholder for future initialization
    }
}

void Match::update() {
    if (state == IN_PROGRESS) {
        // Update match logic
    }
}

void Match::end() {
    state = ENDED;
}

bool Match::isActive() const {
    return state == IN_PROGRESS;
}

void Match::addPlayer(Player* player) {
    players.push_back(player);
}

void Match::removePlayer(Player* player) {
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (*it == player) {
            players.erase(it);
            break;
        }
    }
}

