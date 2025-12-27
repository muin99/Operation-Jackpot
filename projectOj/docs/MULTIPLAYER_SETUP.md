# Multiplayer Setup Guide

## Overview

This game now supports local network multiplayer using **ENet** (a lightweight UDP networking library). The game can run in three modes:
- **Single Player**: Offline mode (default)
- **Server Mode**: Host a game that other players can join
- **Client Mode**: Connect to a server to play with others

## Architecture

### Server-Client Model
- **Server**: Authoritative game state, manages rooms, matches, and synchronizes all players and bullets
- **Client**: Sends input to server, receives game state updates, renders local and remote players

### Network Protocol
The game uses a custom message protocol over ENet UDP with the following message types:
- Connection management (connect, disconnect, accept)
- Room management (create, join, list)
- Match management (start, state updates)
- Game state (player input, player positions, bullet spawn/update/remove)

## Setup Instructions

### 1. Download and Install ENet

#### Windows:
1. Download ENet from: http://enet.bespin.org/Download.html
2. Extract to a folder (e.g., `C:\enet` or `../enet` relative to your project)
3. Build ENet:
   ```bash
   cd enet
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```
4. Copy the built library:
   - Copy `enet.lib` (or `libenet.a`) to `../enet/lib/`
   - Copy all header files from `include/` to `../enet/include/enet/`

#### Alternative: Pre-built ENet
- You can also download pre-built Windows binaries if available
- Place them in `../enet/lib/` and `../enet/include/enet/`

### 2. Update Project Configuration

The project file (`projectOj.cbp`) has been updated to include:
- ENet include directory: `../enet/include`
- ENet library: `enet`
- ENet library directory: `../enet/lib`

**Note**: Adjust the paths in `projectOj.cbp` if your ENet installation is in a different location.

### 3. Project Structure

After setup, your directory structure should look like:
```
projectOj/
├── include/
│   ├── NetworkManager.h
│   ├── NetworkMessages.h
│   └── ...
├── src/
│   ├── NetworkManager.cpp
│   └── ...
└── ../enet/          (or wherever you installed ENet)
    ├── include/
    │   └── enet/
    │       └── enet.h
    └── lib/
        └── enet.lib
```

## How to Use

### Starting a Server

1. Run the game
2. In the main menu, press **S** to start server mode
3. The game will display your server IP address (e.g., `192.168.1.100:7777`)
4. Share this IP address with other players on your local network
5. The server will wait for clients to connect

### Connecting as a Client

1. Run the game on another PC on the same local network
2. In the main menu, press **L** to connect as a client
3. Currently, it connects to `127.0.0.1` (localhost) by default
4. **To connect to a different server**, you'll need to modify the code in `Game.cpp`:
   ```cpp
   // In keyPressed function, change:
   initializeNetwork(false, "127.0.0.1");
   // To:
   initializeNetwork(false, "192.168.1.100");  // Server's IP address
   ```

### Future Enhancement: IP Input
A better solution would be to add an input dialog for the server IP. This can be added later.

### Playing Multiplayer

1. **Server**: Press **C** to create a room, then **S** to start the match
2. **Clients**: Press **J** to join a room (currently joins first available room)
3. Once in a room, the server can start the match
4. All players will see each other in the game
5. Movement and shooting are synchronized across all clients

## Network Features

### Synchronized Elements
- ✅ Player positions and rotations
- ✅ Bullet spawns and positions
- ✅ Room creation and joining
- ✅ Match start/end

### Network Optimization
- Player input is sent unreliably (UDP) for low latency
- Game state updates are sent at ~30 updates/second
- Bullet spawns are sent reliably to ensure all clients see them

## Troubleshooting

### Cannot Connect to Server

1. **Check Firewall**: Windows Firewall may block the connection
   - Add an exception for port 7777 (UDP)
   - Or temporarily disable firewall for testing

2. **Check IP Address**: Ensure you're using the correct server IP
   - Server IP is shown in the menu when server starts
   - Use `ipconfig` (Windows) to find the server's local IP

3. **Check Network**: Ensure both PCs are on the same local network
   - Same router/switch
   - Can ping each other

### Server Not Starting

1. **Port Already in Use**: Another application may be using port 7777
   - Change the port in `Game::initializeNetwork()` or `NetworkManager::startServer()`

2. **ENet Not Found**: Ensure ENet is properly installed and paths are correct
   - Check `projectOj.cbp` for correct include/library paths

### Players Not Visible

1. **Check Connection**: Ensure client is connected (status shown in menu)
2. **Check Server**: Server must be running and in a match
3. **Check Network**: Network messages may be blocked

## Code Structure

### Key Files

- **`include/NetworkManager.h`** & **`src/NetworkManager.cpp`**: Core networking functionality
- **`include/NetworkMessages.h`**: Message type definitions
- **`include/Game.h`** & **`src/Game.cpp`**: Game integration with networking

### Network Flow

1. **Client Input** → `Game::sendPlayerInput()` → `NetworkManager::sendToServer()`
2. **Server Receives** → `NetworkManager::handleMessage()` → Game logic
3. **Server Updates** → `Game::updateServerGameState()` → `NetworkManager::broadcastToAll()`
4. **Clients Receive** → `NetworkManager::handleMessage()` → `Game::handleNetworkPlayerState()`

## Future Improvements

- [ ] IP address input dialog for clients
- [ ] Room list display
- [ ] Player names
- [ ] Latency/ping display
- [ ] Client-side prediction for smoother movement
- [ ] Lag compensation
- [ ] Reconnection handling
- [ ] Redis integration (if needed for persistent storage)

## Notes on Redis

You mentioned "radiis" (likely Redis). For a local network game, **Redis is not necessary**. ENet handles all the networking you need. Redis would only be useful if you want:
- Persistent room/match data across server restarts
- Cross-server communication
- Database storage for player stats

For a simple local network game, ENet alone is sufficient and more efficient.

## Testing

1. **Local Testing**: Run server and client on the same PC (use `127.0.0.1`)
2. **Network Testing**: Run server on one PC, client on another
3. **Multiple Clients**: Connect multiple clients to test synchronization

## Port Configuration

Default port: **7777** (UDP)

To change the port, modify:
- Server: `Game::initializeNetwork(true, "", 7777)` → change `7777`
- Client: `NetworkManager::connectToServer(hostname, 7777)` → change `7777`

Make sure both use the same port!

