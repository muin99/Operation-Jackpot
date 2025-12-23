#pragma once

#include <vector>

class Player;
class Map;
class Room;

class Match {
public:
    enum MatchState {
        PREPARING,    // Match is being set up
        COUNTDOWN,    // Countdown before start
        IN_PROGRESS,  // Match is active
        ENDED         // Match has finished
    };

    int matchId;
    MatchState state;
    Map* gameMap;
    std::vector<Player*> players;
    Room* sourceRoom;

    Match(int id, Room* room);
    ~Match();

    void start();
    void update();
    void end();
    bool isActive() const;
    void addPlayer(Player* player);
    void removePlayer(Player* player);
};

