#include "Game.h"
#include "Player.h"
#include "Map.h"
#include "Room.h"
#include "Match.h"
#include "Bullet.h"
#include "Sound.h"
#include "NetworkManager.h"
#include "NetworkMessages.h"
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <enet/enet.h>

int Game::width = 0;
int Game::height = 0;
Game::MenuState Game::menuState = Game::NONE;
Player* Game::currentPlayer = nullptr;
Map* Game::gameMap = nullptr;
Room* Game::currentRoom = nullptr;
Match* Game::currentMatch = nullptr;
std::vector<Room*> Game::rooms;
std::vector<Bullet*> Game::bullets;
std::map<int, Player*> Game::remotePlayers;
std::map<int, Bullet*> Game::networkBullets;
std::vector<RoomInfoMessage> Game::availableRooms;
NetworkManager* Game::networkManager = nullptr;
bool Game::isServer = false;
bool Game::isClient = false;
std::string Game::serverIP = "";
int Game::serverPort = 7777;
bool Game::autoStartServerFlag = false;
bool Game::autoStartClientFlag = false;
std::string Game::autoStartClientIP = "127.0.0.1";
float Game::mouseX = 0.0f;
float Game::mouseY = 0.0f;
float Game::cameraX = 0.0f;
float Game::cameraY = 0.0f;
bool Game::lastShootState = false;
float Game::lastInputSendTime = 0.0f;

Game::Game(int w, int h, int argc, char** argv) {
    width = w;
    height = h;
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--server" || arg == "-s") {
            autoStartServerFlag = true;
        } else if (arg == "--client" || arg == "-c") {
            autoStartClientFlag = true;
            // Check if next argument is an IP address
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                autoStartClientIP = argv[i + 1];
                i++; // Skip the IP argument
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: projectOj.exe [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --server, -s          Start as server\n";
            std::cout << "  --client, -c [IP]     Connect as client (default IP: 127.0.0.1)\n";
            std::cout << "  --help, -h           Show this help message\n";
            std::cout << "\nExamples:\n";
            std::cout << "  projectOj.exe --server\n";
            std::cout << "  projectOj.exe --client 192.168.0.100\n";
            // Note: glutMainLoop never returns, so we can't exit here
            // But at least the help will be printed
        }
    }
    
    runOpenGl(argc, argv);
}

void Game::runOpenGl(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Operation Jackpot");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutTimerFunc(16, timer, 0);  // ~60 FPS update rate
    glutKeyboardFunc(keyPressed);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyPressed);
    glutSpecialUpFunc(specialKeyUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);

    glutMainLoop();
}

void Game::display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);

    if (menuState == NONE) {
        drawText(300, 450, "S - Start Server");
        drawText(300, 400, "L - Connect as Client");
        drawText(300, 350, "J - Join Room");
        drawText(300, 300, "C - Create Room");
        if (isServer) {
            drawText(250, 200, "SERVER MODE - IP: " + NetworkManager::getLocalIP() + ":" + std::to_string(serverPort));
        } else if (isClient) {
            drawText(250, 200, "CLIENT MODE - Connected to: " + serverIP);
        }
    }
    else if (menuState == ROOM_LIST) {
        drawText(280, 500, "Available Rooms:");
        drawText(250, 470, "Press number key to join (1-9)");
        drawText(250, 440, "R - Refresh list");
        drawText(250, 410, "ESC - Back to menu");
        
        float yPos = 380.0f;
        for (size_t i = 0; i < availableRooms.size() && i < 9; i++) {
            const RoomInfoMessage& room = availableRooms[i];
            std::string roomText = std::to_string(i + 1) + ". " + std::string(room.roomName) + 
                                   " (" + std::to_string(room.playerCount) + "/" + 
                                   std::to_string(room.maxPlayers) + ")";
            if (room.status == (int)Room::WAITING) {
                roomText += " [WAITING]";
            } else if (room.status == (int)Room::IN_MATCH) {
                roomText += " [IN MATCH]";
            }
            drawText(250, yPos, roomText);
            yPos -= 30.0f;
        }
        
        if (availableRooms.empty()) {
            drawText(250, 350, "No rooms available");
            drawText(250, 320, "Press C to create a room");
        }
    }
    else if (menuState == JOIN_ROOM) {
        drawText(280, 400, "Joining Room...");
        drawText(250, 370, "Please wait...");
        drawText(250, 340, "ESC - Cancel");
    }
    else if (menuState == IN_ROOM) {
        if (currentRoom != nullptr) {
            drawText(280, 450, "Room: " + currentRoom->roomName);
            drawText(250, 420, "Players: " + std::to_string(currentRoom->getPlayerCount()) + 
                      "/" + std::to_string(currentRoom->maxPlayers));
            
            // Show player list
            float yPos = 380.0f;
            drawText(250, yPos, "Players in room:");
            yPos -= 25.0f;
            for (size_t i = 0; i < currentRoom->players.size() && i < 8; i++) {
                Player* p = currentRoom->players[i];
                if (p != nullptr) {
                    std::string playerText = "  Player " + std::to_string(p->id);
                    if (p == currentPlayer || (isClient && p->id == networkManager->localPlayerId)) {
                        playerText += " (You)";
                    }
                    drawText(250, yPos, playerText);
                    yPos -= 20.0f;
                }
            }
            
            // Show controls
            if (isServer || (currentRoom->getPlayerCount() > 0)) {
                drawText(250, yPos - 10, "S - Start Match");
            }
            drawText(250, yPos - 40, "ESC - Leave Room");
        } else {
            drawText(280, 400, "Waiting for room info...");
        }
    }
    else if (menuState == PLAYING) {
        // Update camera to follow player (center player on screen)
        if (currentPlayer != nullptr) {
            // Camera offset is the player's position minus screen center
            cameraX = currentPlayer->x - width / 2.0f;
            cameraY = currentPlayer->y - height / 2.0f;
        }
        
        // Apply camera transformation (translate world so player appears at center)
        glPushMatrix();
        glTranslatef(-cameraX, -cameraY, 0.0f);
        
        // Render map (will be offset by camera)
        if (gameMap != nullptr) {
            gameMap->render();
        }
        
        // Render all bullets (will be offset by camera)
        // Render local bullets (server's own bullets or local game)
        for (Bullet* bullet : bullets) {
            if (bullet != nullptr && bullet->active) {
                bullet->render();
            }
        }
        
        // Render network bullets (bullets from other players)
        for (auto& pair : networkBullets) {
            if (pair.second != nullptr && pair.second->active) {
                pair.second->render();
            }
        }
        
        // Render remote players (will be offset by camera)
        // First render from remotePlayers map
        for (auto& pair : remotePlayers) {
            if (pair.second != nullptr) {
                // Skip rendering if this is our own player (we render it separately)
                bool isLocalPlayer = false;
                if (isClient && networkManager != nullptr) {
                    isLocalPlayer = (pair.second->id == networkManager->localPlayerId);
                } else if (currentPlayer != nullptr) {
                    isLocalPlayer = (pair.second->id == currentPlayer->id);
                }
                
                if (!isLocalPlayer) {
                    glPushMatrix();
                    glTranslatef(pair.second->x, pair.second->y, 0.0f);
                    glRotatef(pair.second->angle * 180.0f / 3.14159f, 0.0f, 0.0f, 1.0f);
                    
                    // Different color for remote players
                    glColor3f(1.0f, 0.0f, 0.0f);  // Red for remote players
                    glBegin(GL_TRIANGLES);
                    glVertex2f(0, pair.second->size / 2);
                    glVertex2f(-pair.second->size / 2, -pair.second->size / 2);
                    glVertex2f(pair.second->size / 2, -pair.second->size / 2);
                    glEnd();
                    
                    glPopMatrix();
                }
            }
        }
        
        // Also render players from current room (server side, in case they're not in remotePlayers)
        if (isServer && currentRoom != nullptr) {
            for (Player* roomPlayer : currentRoom->players) {
                if (roomPlayer != nullptr) {
                    // Skip if this is our own player
                    bool isLocalPlayer = (currentPlayer != nullptr && roomPlayer->id == currentPlayer->id);
                    // Skip if already rendered via remotePlayers
                    bool alreadyRendered = (remotePlayers.find(roomPlayer->id) != remotePlayers.end());
                    
                    if (!isLocalPlayer && !alreadyRendered) {
                        glPushMatrix();
                        glTranslatef(roomPlayer->x, roomPlayer->y, 0.0f);
                        glRotatef(roomPlayer->angle * 180.0f / 3.14159f, 0.0f, 0.0f, 1.0f);
                        
                        // Different color for remote players
                        glColor3f(1.0f, 0.0f, 0.0f);  // Red for remote players
                        glBegin(GL_TRIANGLES);
                        glVertex2f(0, roomPlayer->size / 2);
                        glVertex2f(-roomPlayer->size / 2, -roomPlayer->size / 2);
                        glVertex2f(roomPlayer->size / 2, -roomPlayer->size / 2);
                        glEnd();
                        
                        glPopMatrix();
                    }
                }
            }
        }
        
        // Render player at screen center (no camera offset)
        glPopMatrix();
        if (currentPlayer != nullptr) {
            // Render player at fixed screen position (center)
            glPushMatrix();
            glTranslatef(width / 2.0f, height / 2.0f, 0.0f);
            glRotatef(currentPlayer->angle * 180.0f / 3.14159f, 0.0f, 0.0f, 1.0f);
            
            glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(0, currentPlayer->size / 2);
            glVertex2f(-currentPlayer->size / 2, -currentPlayer->size / 2);
            glVertex2f(currentPlayer->size / 2, -currentPlayer->size / 2);
            glEnd();
            
            glPopMatrix();
        }
        
        // Draw crosshair cursor at mouse position (screen coordinates, no camera offset)
        drawCrosshair(mouseX, mouseY);
    }

    glutSwapBuffers();
}

void Game::idle() {
    // Keep rendering smooth
    if (menuState == PLAYING) {
        glutPostRedisplay();
    }
}

void Game::timer(int value) {
    // Auto-start network mode if requested via command-line
    static bool autoStartHandled = false;
    if (!autoStartHandled) {
        if (autoStartServerFlag) {
            initializeNetwork(true);
            std::cout << "Server started! IP: " << NetworkManager::getLocalIP() << ":" << serverPort << "\n";
            autoStartHandled = true;
        } else if (autoStartClientFlag) {
            initializeNetwork(false, autoStartClientIP);
            std::cout << "Connecting to server at " << autoStartClientIP << ":" << serverPort << "...\n";
            autoStartHandled = true;
        }
    }
    
    // Update network
    updateNetwork();
    
    if (menuState == PLAYING) {
        // SERVER: Process all player movements (authoritative)
        if (isServer) {
            // Process server's own player if it exists
            if (currentPlayer != nullptr) {
                currentPlayer->updateMovement();
            }
            
            // Process all remote players (clients)
            for (auto& pair : remotePlayers) {
                if (pair.second != nullptr) {
                    pair.second->updateMovement();
                }
            }
        }
        // CLIENT: Process own player locally (client-side prediction)
        // Remote players are updated from server state
        else if (isClient) {
            // Process own player movement locally for responsiveness
            if (currentPlayer != nullptr) {
                currentPlayer->updateMovement();
            }
        }
        // LOCAL (non-networked): Process movement normally
        else {
            if (currentPlayer != nullptr) {
                currentPlayer->updateMovement();
            }
        }
        
        updateBullets();
        cleanupBullets();
        
        // Send player input to server (if client)
        if (isClient && networkManager != nullptr && networkManager->isConnected) {
            sendPlayerInput();
        }
        
        // Update server game state (if server)
        if (isServer && networkManager != nullptr) {
            updateServerGameState();
        }
        
        glutPostRedisplay();
    }
    // Restart timer for continuous updates (~60 FPS)
    glutTimerFunc(16, timer, 0);
}

void Game::keyPressed(unsigned char key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->handleKey(key, true);
        }
        if (key == 27) {
            menuState = NONE;
            // Show cursor when leaving game
            glutSetCursor(GLUT_CURSOR_INHERIT);
            // Clean up all bullets
            for (Bullet* bullet : bullets) {
                if (bullet != nullptr) {
                    delete bullet;
                }
            }
            bullets.clear();
        }
    }
        else if (menuState == IN_ROOM) {
        if (key == 's' || key == 'S') {
            // Start the match
            if (isClient && networkManager != nullptr && networkManager->isConnected) {
                networkManager->sendStartMatchRequest();
            } else if (isServer) {
                // Server starting match - need to broadcast to all clients
                startMatch();
                if (currentMatch != nullptr && currentMatch->isActive()) {
                    menuState = PLAYING;
                    std::cout << "Match started!\n";
                    
                    // Broadcast match start to all clients in the room
                    if (currentRoom != nullptr && networkManager != nullptr) {
                        StartMatchResponseMessage response;
                        response.success = true;
                        response.matchId = currentMatch->matchId;
                        
                        MessageType msgType = MessageType::START_MATCH_RESPONSE;
                        
                        // Send to all players in the room
                        int sentCount = 0;
                        for (Player* p : currentRoom->players) {
                            if (p != nullptr) {
                                auto it = networkManager->playerIdToPeer.find(p->id);
                                if (it != networkManager->playerIdToPeer.end() && it->second != nullptr) {
                                    ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                                    enet_packet_resize(packet, sizeof(MessageType) + sizeof(StartMatchResponseMessage));
                                    memcpy(packet->data + sizeof(MessageType), &response, sizeof(StartMatchResponseMessage));
                                    enet_peer_send(it->second, 0, packet);
                                    sentCount++;
                                }
                            }
                        }
                        std::cout << "Broadcasted match start to " << sentCount << " clients\n";
                    }
                }
            } else {
                // Local (non-networked) match start
                startMatch();
                if (currentMatch != nullptr && currentMatch->isActive()) {
                    menuState = PLAYING;
                    std::cout << "Match started!\n";
                }
            }
        }
        else if (key == 27) {
            // Leave room
            if (isClient && networkManager != nullptr && networkManager->isConnected) {
                // TODO: Send leave room request to server
            }
            currentRoom = nullptr;
            menuState = NONE;
            std::cout << "Left room\n";
        }
    }
    else if (menuState == JOIN_ROOM) {
        if (key == 27) {  // ESC to cancel
            menuState = ROOM_LIST;
        }
    }
    else if (menuState == ROOM_LIST) {
        // Handle room selection by number keys (1-9)
        if (key >= '1' && key <= '9') {
            int roomIndex = key - '1';
            if (roomIndex < (int)availableRooms.size()) {
                int roomId = availableRooms[roomIndex].roomId;
                if (isClient && networkManager != nullptr && networkManager->isConnected) {
                    networkManager->sendJoinRoomRequest(roomId);
                    menuState = JOIN_ROOM;
                } else {
                    // Local join
                    if (joinRoom(roomId)) {
                        menuState = IN_ROOM;
                    }
                }
            }
        }
        else if (key == 'r' || key == 'R') {
            // Refresh room list
            requestRoomList();
        }
        else if (key == 27) {  // ESC
            menuState = NONE;
        }
    }
    else {
        if (key == 'j' || key == 'J') {
            // Request room list
            requestRoomList();
            menuState = ROOM_LIST;
        }
        else if (key == 'c' || key == 'C') {
            // Create a room
            if (currentPlayer == nullptr) {
                currentPlayer = new Player(1, width/2, height/2);
            }
            
            // If networked, send create room request
            if (isClient && networkManager != nullptr && networkManager->isConnected) {
                networkManager->sendCreateRoomRequest("Room " + std::to_string(rooms.size() + 1), 4);
            } else {
                // Local room creation
                Room* newRoom = createRoom("Room " + std::to_string(rooms.size() + 1), 4);
                if (newRoom != nullptr && currentPlayer != nullptr) {
                    newRoom->addPlayer(currentPlayer);
                }
                menuState = IN_ROOM;
                std::cout << "Room created! Press S to start match.\n";
            }
        }
        else if (key == 's' || key == 'S') {
            // Start server mode
            if (!isServer && !isClient) {
                initializeNetwork(true);
                std::cout << "Server started! IP: " << NetworkManager::getLocalIP() << ":" << serverPort << "\n";
            }
        }
        else if (key == 'l' || key == 'L') {
            // Connect as client (localhost for now, can be changed)
            if (!isServer && !isClient) {
                std::cout << "Enter server IP (or press Enter for localhost): ";
                // For now, use localhost - can be enhanced with input dialog
                initializeNetwork(false, "127.0.0.1");
            }
        }
        else if (key == 27) {
            menuState = NONE;
            std::cout << "Back to menu\n";
        }
    }

    glutPostRedisplay();
}

void Game::keyUp(unsigned char key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->handleKey(key, false);
        }
    }
}

void Game::specialKeyPressed(int key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->handleSpecialKey(key, true);
        }
        glutPostRedisplay();
    }
}

void Game::specialKeyUp(int key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->handleSpecialKey(key, false);
        }
    }
}

void Game::mouseMotion(int x, int y) {
    // Always update mouse position (screen coordinates)
    mouseX = x;
    mouseY = height - y;  // Flip Y coordinate (GLUT has origin at top-left, OpenGL at bottom-left)
    
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            // Convert screen mouse position to world coordinates for aiming
            // Since player is at screen center, mouse position relative to center
            // gives us the direction vector
            float worldMouseX = currentPlayer->x + (mouseX - width / 2.0f);
            float worldMouseY = currentPlayer->y + (mouseY - height / 2.0f);
            
            currentPlayer->updateAim(worldMouseX, worldMouseY);
            glutPostRedisplay();
        }
    }
}

void Game::mouseClick(int button, int state, int x, int y) {
    if (menuState == PLAYING && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentPlayer != nullptr) {
            // Convert screen mouse coordinates to world coordinates
            float screenMouseX = x;
            float screenMouseY = height - y;
            float worldMouseX = currentPlayer->x + (screenMouseX - width / 2.0f);
            float worldMouseY = currentPlayer->y + (screenMouseY - height / 2.0f);
            
            // CLIENT: Only send shoot input to server (server creates bullet)
            if (isClient && networkManager != nullptr && networkManager->isConnected) {
                // Send shoot input - server will create bullet and broadcast it
                bool w = currentPlayer->keys['w'] || currentPlayer->keys['W'];
                bool s = currentPlayer->keys['s'] || currentPlayer->keys['S'];
                bool a = currentPlayer->keys['a'] || currentPlayer->keys['A'];
                bool d = currentPlayer->keys['d'] || currentPlayer->keys['D'];
                networkManager->sendPlayerInput(w, s, a, d, worldMouseX, worldMouseY, true);
            } 
            // SERVER: Create bullet directly (authoritative)
            else if (isServer && currentPlayer != nullptr) {
                float spawnX, spawnY;
                currentPlayer->getBulletSpawnPosition(spawnX, spawnY);
                
                Bullet* newBullet = new Bullet(spawnX, spawnY, currentPlayer->angle, currentPlayer->id);
                bullets.push_back(newBullet);
                
                // Broadcast bullet spawn to all clients
                if (networkManager != nullptr) {
                    BulletSpawnMessage msg;
                    msg.bulletId = (int)(intptr_t)newBullet;  // Use pointer as ID
                    msg.x = spawnX;
                    msg.y = spawnY;
                    msg.vx = newBullet->vx;
                    msg.vy = newBullet->vy;
                    msg.ownerId = currentPlayer->id;
                    networkManager->broadcastToAll(MessageType::BULLET_SPAWN, &msg, sizeof(msg), true);
                }
            }
            // LOCAL (non-networked): Create bullet directly
            else if (!isServer && !isClient && currentPlayer != nullptr) {
                float spawnX, spawnY;
                currentPlayer->getBulletSpawnPosition(spawnX, spawnY);
                Bullet* newBullet = new Bullet(spawnX, spawnY, currentPlayer->angle, currentPlayer->id);
                bullets.push_back(newBullet);
            }
            
            // Play gunshot sound
            Sound::playGunshot();
            
            glutPostRedisplay();
        }
    }
}

void Game::drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void Game::drawCrosshair(float x, float y) {
    float size = 8.0f;  // Size of crosshair lines
    float thickness = 2.0f;  // Thickness of crosshair lines
    
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    glLineWidth(thickness);
    
    glBegin(GL_LINES);
    // Horizontal line
    glVertex2f(x - size, y);
    glVertex2f(x + size, y);
    // Vertical line
    glVertex2f(x, y - size);
    glVertex2f(x, y + size);
    glEnd();
    
    // Draw center dot
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
    
    glLineWidth(1.0f);  // Reset line width
}

void Game::updateBullets() {
    // Update local bullets (server's own bullets or local game)
    for (Bullet* bullet : bullets) {
        if (bullet != nullptr) {
            bullet->update();
            // Deactivate bullets that go out of bounds
            if (bullet->isOutOfBounds(width, height)) {
                bullet->active = false;
            }
        }
    }
    
    // Update network bullets (bullets from other players)
    for (auto& pair : networkBullets) {
        if (pair.second != nullptr) {
            pair.second->update();
            // Deactivate bullets that go out of bounds
            if (pair.second->isOutOfBounds(width, height)) {
                pair.second->active = false;
            }
        }
    }
}

void Game::cleanupBullets() {
    // Remove inactive local bullets
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](Bullet* bullet) {
                if (bullet != nullptr && !bullet->active) {
                    delete bullet;
                    return true;
                }
                return false;
            }),
        bullets.end()
    );
    
    // Remove inactive network bullets
    for (auto it = networkBullets.begin(); it != networkBullets.end();) {
        if (it->second != nullptr && !it->second->active) {
            delete it->second;
            it = networkBullets.erase(it);
        } else {
            ++it;
        }
    }
}

Room* Game::createRoom(const std::string& roomName, int maxPlayers) {
    int newRoomId = rooms.size() + 1;
    Room* newRoom = new Room(newRoomId, roomName, maxPlayers);
    rooms.push_back(newRoom);
    currentRoom = newRoom;
    return newRoom;
}

bool Game::joinRoom(int roomId) {
    for (Room* room : rooms) {
        if (room->roomId == roomId && room->canJoin()) {
            if (currentPlayer == nullptr) {
                currentPlayer = new Player(1, width/2, height/2);
            }
            if (room->addPlayer(currentPlayer)) {
                currentRoom = room;
                return true;
            }
        }
    }
    return false;
}

void Game::startMatch() {
    if (currentRoom == nullptr) {
        std::cout << "Cannot start match: currentRoom is null\n";
        return;
    }
    
    if (currentRoom->status != Room::WAITING) {
        std::cout << "Cannot start match: room status is " << (int)currentRoom->status << " (expected WAITING=" << (int)Room::WAITING << ")\n";
        return;
    }
    
    if (currentRoom != nullptr && currentRoom->status == Room::WAITING) {
        // Initialize game map if not already created
        if (gameMap == nullptr) {
            gameMap = new Map(width, height);
        }
        
        // SERVER: Initialize remote players from room players
        if (isServer) {
            for (Player* roomPlayer : currentRoom->players) {
                if (roomPlayer != nullptr) {
                    // If this is not the server's own player, add to remotePlayers
                    if (currentPlayer == nullptr || roomPlayer->id != currentPlayer->id) {
                        if (remotePlayers.find(roomPlayer->id) == remotePlayers.end()) {
                            remotePlayers[roomPlayer->id] = roomPlayer;
                            std::cout << "[SERVER] Added player " << roomPlayer->id << " to remotePlayers from room\n";
                        }
                    }
                }
            }
        }
        
        // Ensure player exists
        if (currentPlayer == nullptr) {
            // Try to get player from room if available
            if (!currentRoom->players.empty()) {
                currentPlayer = currentRoom->players[0];
            } else {
                // Create a new player
                int playerId = (isClient && networkManager != nullptr) ? networkManager->localPlayerId : 1;
                currentPlayer = new Player(playerId, width/2, height/2);
            }
        }
        
        currentRoom->setStatus(Room::STARTING);
        int matchId = currentRoom->roomId; // Use room ID as match ID
        currentMatch = new Match(matchId, currentRoom);
        // Set the match's gameMap to the Game's gameMap
        currentMatch->gameMap = gameMap;
        currentMatch->start();
        currentRoom->setStatus(Room::IN_MATCH);
        
        // Initialize mouse position to center of screen
        mouseX = width / 2.0f;
        mouseY = height / 2.0f;
        
        // Clear any existing bullets
        for (Bullet* bullet : bullets) {
            if (bullet != nullptr) {
                delete bullet;
            }
        }
        bullets.clear();
        
        // Hide system cursor when playing
        glutSetCursor(GLUT_CURSOR_NONE);
        
        // Enter playing state
        menuState = PLAYING;
        std::cout << "Match started! Room " << currentRoom->roomId << " with " << currentRoom->getPlayerCount() << " players\n";
    } else {
        std::cout << "Cannot start match: room is " << (currentRoom == nullptr ? "null" : "not in WAITING state") << "\n";
    }
}

// Network functions implementation
void Game::initializeNetwork(bool asServer, const std::string& serverIP, int port) {
    if (networkManager == nullptr) {
        networkManager = new NetworkManager();
        if (!networkManager->initialize()) {
            std::cerr << "Failed to initialize network\n";
            delete networkManager;
            networkManager = nullptr;
            return;
        }
    }
    
    serverPort = port;
    Game::serverIP = serverIP;
    
    if (asServer) {
        if (networkManager->startServer(port)) {
            isServer = true;
            isClient = false;
            
            // Set up server callbacks
            networkManager->onPlayerStateUpdate = nullptr;  // Server doesn't receive player states
            networkManager->onBulletSpawn = nullptr;
            networkManager->onBulletRemove = nullptr;
            networkManager->onRoomUpdate = nullptr;
            networkManager->onJoinRoomResponse = nullptr;
            networkManager->onCreateRoomResponse = nullptr;
            networkManager->onMatchStart = nullptr;
            networkManager->onPlayerInput = [](int playerId, bool w, bool s, bool a, bool d, float mouseX, float mouseY, bool shoot) {
                // Handle player input on server - ONLY update remotePlayers, never currentPlayer
                // currentPlayer is for the server's own player (if server is playing)
                Player* player = nullptr;
                
                // Always use remotePlayers for client input (clients are always remote players)
                if (remotePlayers.find(playerId) != remotePlayers.end()) {
                    player = remotePlayers[playerId];
                } else {
                    // Create new remote player if doesn't exist
                    player = new Player(playerId, width/2, height/2);
                    remotePlayers[playerId] = player;
                    std::cout << "[SERVER] Created remote player " << playerId << " for input\n";
                }
                
                if (player != nullptr) {
                    // Update player input state
                    player->keys['w'] = player->keys['W'] = w;
                    player->keys['s'] = player->keys['S'] = s;
                    player->keys['a'] = player->keys['A'] = a;
                    player->keys['d'] = player->keys['D'] = d;
                    
                    // Update aim (convert from world coordinates if needed)
                    player->updateAim(mouseX, mouseY);
                    
                    // Handle shooting - server is authoritative
                    if (shoot) {
                        float spawnX, spawnY;
                        player->getBulletSpawnPosition(spawnX, spawnY);
                        Bullet* newBullet = new Bullet(spawnX, spawnY, player->angle, player->id);
                        bullets.push_back(newBullet);
                        
                        // Broadcast bullet spawn to all clients
                        if (networkManager != nullptr) {
                            BulletSpawnMessage msg;
                            msg.bulletId = (int)(intptr_t)newBullet;  // Use pointer as ID
                            msg.x = spawnX;
                            msg.y = spawnY;
                            msg.vx = newBullet->vx;
                            msg.vy = newBullet->vy;
                            msg.ownerId = player->id;
                            networkManager->broadcastToAll(MessageType::BULLET_SPAWN, &msg, sizeof(msg), true);
                        }
                        
                        Sound::playGunshot();
                    }
                }
            };
        }
    } else {
        if (networkManager->connectToServer(serverIP, port)) {
            isClient = true;
            isServer = false;
            
            // Set up client callbacks
            networkManager->onPlayerStateUpdate = handleNetworkPlayerState;
            networkManager->onBulletSpawn = handleNetworkBulletSpawn;
            networkManager->onBulletUpdate = [](int bulletId, float x, float y) {
                // Update network bullet position from server
                if (networkBullets.find(bulletId) != networkBullets.end()) {
                    Bullet* bullet = networkBullets[bulletId];
                    if (bullet != nullptr) {
                        bullet->x = x;
                        bullet->y = y;
                    }
                }
            };
            networkManager->onBulletRemove = handleNetworkBulletRemove;
            networkManager->onRoomUpdate = handleNetworkRoomUpdate;
            networkManager->onRoomListResponse = handleNetworkRoomListResponse;
            networkManager->onJoinRoomResponse = handleNetworkJoinRoomResponse;
            networkManager->onCreateRoomResponse = handleNetworkCreateRoomResponse;
            networkManager->onMatchStart = handleNetworkMatchStart;
        }
    }
}

void Game::shutdownNetwork() {
    if (networkManager != nullptr) {
        networkManager->shutdown();
        delete networkManager;
        networkManager = nullptr;
    }
    isServer = false;
    isClient = false;
}

void Game::updateNetwork() {
    if (networkManager != nullptr) {
        networkManager->update();
    }
}

void Game::sendPlayerInput() {
    if (networkManager == nullptr || currentPlayer == nullptr) return;
    
    // Send input more frequently for better responsiveness (~60 updates per second)
    static float lastInputSendTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    
    if (currentTime - lastInputSendTime < 0.016f) {
        return;  // Throttle to ~60 updates per second
    }
    lastInputSendTime = currentTime;
    
    bool w = currentPlayer->keys['w'] || currentPlayer->keys['W'];
    bool s = currentPlayer->keys['s'] || currentPlayer->keys['S'];
    bool a = currentPlayer->keys['a'] || currentPlayer->keys['A'];
    bool d = currentPlayer->keys['d'] || currentPlayer->keys['D'];
    
    // Convert mouse position to world coordinates
    float worldMouseX = currentPlayer->x + (mouseX - width / 2.0f);
    float worldMouseY = currentPlayer->y + (mouseY - height / 2.0f);
    
    bool shoot = false;  // Will be set when mouse is clicked
    
    networkManager->sendPlayerInput(w, s, a, d, worldMouseX, worldMouseY, shoot);
}

void Game::handleNetworkPlayerState(int playerId, float x, float y, float angle) {
    // CLIENT: Update remote players from server state (authoritative)
    // Skip our own player - we use client-side prediction for our own player
    
    // Check if this is our own player
    bool isLocalPlayer = false;
    if (isClient && networkManager != nullptr) {
        isLocalPlayer = (playerId == networkManager->localPlayerId);
    } else if (currentPlayer != nullptr) {
        isLocalPlayer = (playerId == currentPlayer->id);
    }
    
    // Skip updating our own player (client-side prediction)
    if (isLocalPlayer) {
        return;
    }
    
    // Update or create remote player
    if (remotePlayers.find(playerId) == remotePlayers.end()) {
        remotePlayers[playerId] = new Player(playerId, x, y);
        std::cout << "[CLIENT] Created remote player " << playerId << " at (" << x << ", " << y << ")\n";
    }
    
    Player* remotePlayer = remotePlayers[playerId];
    if (remotePlayer != nullptr) {
        // Update position and angle from server (authoritative)
        remotePlayer->x = x;
        remotePlayer->y = y;
        remotePlayer->angle = angle;
    }
}

void Game::handleNetworkBulletSpawn(int bulletId, float x, float y, float vx, float vy, int ownerId) {
    // Create network bullet (from server)
    // Don't create if it's our own bullet (we might have created it locally)
    bool isOwnBullet = false;
    if (isClient && networkManager != nullptr) {
        isOwnBullet = (ownerId == networkManager->localPlayerId);
    }
    
    // Only create if it doesn't exist and it's not our own bullet
    if (networkBullets.find(bulletId) == networkBullets.end() && !isOwnBullet) {
        // Calculate angle from velocity
        float angle = atan2(vy, vx) - 3.14159f / 2.0f;
        Bullet* bullet = new Bullet(x, y, angle, ownerId);
        bullet->vx = vx;
        bullet->vy = vy;
        networkBullets[bulletId] = bullet;
    }
}

void Game::handleNetworkBulletRemove(int bulletId) {
    if (networkBullets.find(bulletId) != networkBullets.end()) {
        Bullet* bullet = networkBullets[bulletId];
        if (bullet != nullptr) {
            delete bullet;
        }
        networkBullets.erase(bulletId);
    }
}

void Game::handleNetworkRoomUpdate(int roomId, const std::string& name, int playerCount, int maxPlayers) {
    // Update room information
    std::cout << "Room update: " << name << " (" << playerCount << "/" << maxPlayers << ")\n";
}

void Game::handleNetworkRoomListResponse(const std::vector<RoomInfoMessage>& roomList) {
    availableRooms = roomList;
    std::cout << "Received room list: " << roomList.size() << " rooms\n";
    // Stay in ROOM_LIST state to display the list
    if (menuState != ROOM_LIST) {
        menuState = ROOM_LIST;
    }
}

void Game::handleNetworkJoinRoomResponse(bool success, int roomId) {
    if (success) {
        // Find the room we joined
        for (Room* room : rooms) {
            if (room != nullptr && room->roomId == roomId) {
                currentRoom = room;
                break;
            }
        }
        
        // If room not found locally, create a placeholder room (server has the real room)
        if (currentRoom == nullptr) {
            // Create a local representation of the room
            // The server has the authoritative version
            currentRoom = new Room(roomId, "Room " + std::to_string(roomId), 4);
            rooms.push_back(currentRoom);
            
            // Add our player to the local room representation
            if (currentPlayer == nullptr && networkManager != nullptr) {
                currentPlayer = new Player(networkManager->localPlayerId, width/2, height/2);
            }
            if (currentPlayer != nullptr) {
                currentRoom->addPlayer(currentPlayer);
            }
        }
        
        std::cout << "Successfully joined room " << roomId << "\n";
        menuState = IN_ROOM;
    } else {
        std::cout << "Failed to join room " << roomId << "\n";
        menuState = ROOM_LIST;  // Go back to room list instead of main menu
    }
}

void Game::handleNetworkCreateRoomResponse(bool success, int roomId) {
    if (success) {
        // Find the room we created
        for (Room* room : rooms) {
            if (room != nullptr && room->roomId == roomId) {
                currentRoom = room;
                break;
            }
        }
        
        // If room not found, create it locally
        if (currentRoom == nullptr) {
            currentRoom = new Room(roomId, "Room " + std::to_string(roomId), 4);
            rooms.push_back(currentRoom);
            
            // Add our player
            if (currentPlayer == nullptr && networkManager != nullptr) {
                currentPlayer = new Player(networkManager->localPlayerId, width/2, height/2);
            }
            if (currentPlayer != nullptr) {
                currentRoom->addPlayer(currentPlayer);
            }
        }
        
        std::cout << "Room created: " << roomId << "\n";
        menuState = IN_ROOM;
    } else {
        std::cout << "Failed to create room\n";
        menuState = NONE;
    }
}

void Game::handleNetworkMatchStart() {
    std::cout << "Received match start signal from server\n";
    
    // Use the same startMatch function to ensure consistency
    if (currentRoom != nullptr) {
        startMatch();
        if (currentMatch != nullptr && currentMatch->isActive()) {
            std::cout << "Client entered match successfully!\n";
        } else {
            std::cout << "Warning: Match started but not active\n";
        }
    } else {
        std::cout << "Error: No current room when match started, creating placeholder...\n";
        // Try to find any room we might be in
        if (!rooms.empty()) {
            currentRoom = rooms[0];
            startMatch();
        } else {
            std::cout << "Fatal: No rooms available\n";
        }
    }
}

void Game::updateServerGameState() {
    if (networkManager == nullptr || !isServer) return;
    
    // Broadcast player states periodically
    static float lastBroadcastTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    
    if (currentTime - lastBroadcastTime > 0.033f) {  // ~30 updates per second
        broadcastPlayerStates();
        broadcastBulletStates();
        lastBroadcastTime = currentTime;
    }
}

void Game::broadcastPlayerStates() {
    if (networkManager == nullptr || !isServer) return;
    
    // Send server's own player state (if server is playing)
    if (currentPlayer != nullptr) {
        PlayerStateMessage msg;
        msg.playerId = currentPlayer->id;  // Use actual player ID
        msg.x = currentPlayer->x;
        msg.y = currentPlayer->y;
        msg.angle = currentPlayer->angle;
        networkManager->broadcastToAll(MessageType::PLAYER_STATE, &msg, sizeof(msg), false);
    }
    
    // Send all remote player states (clients)
    // Also check room players to ensure we don't miss anyone
    for (auto& pair : remotePlayers) {
        if (pair.second != nullptr) {
            PlayerStateMessage msg;
            msg.playerId = pair.first;
            msg.x = pair.second->x;
            msg.y = pair.second->y;
            msg.angle = pair.second->angle;
            networkManager->broadcastToAll(MessageType::PLAYER_STATE, &msg, sizeof(msg), false);
        }
    }
    
    // Also broadcast players from current room (in case they're not in remotePlayers yet)
    if (currentRoom != nullptr) {
        for (Player* roomPlayer : currentRoom->players) {
            if (roomPlayer != nullptr) {
                // Skip if this is the server's own player (already sent above)
                if (currentPlayer == nullptr || roomPlayer->id != currentPlayer->id) {
                    // Skip if already sent via remotePlayers
                    if (remotePlayers.find(roomPlayer->id) == remotePlayers.end()) {
                        PlayerStateMessage msg;
                        msg.playerId = roomPlayer->id;
                        msg.x = roomPlayer->x;
                        msg.y = roomPlayer->y;
                        msg.angle = roomPlayer->angle;
                        networkManager->broadcastToAll(MessageType::PLAYER_STATE, &msg, sizeof(msg), false);
                    }
                }
            }
        }
    }
}

void Game::broadcastBulletStates() {
    if (networkManager == nullptr || !isServer) return;
    
    // Send all bullet states
    for (Bullet* bullet : bullets) {
        if (bullet != nullptr && bullet->active) {
            BulletUpdateMessage msg;
            msg.bulletId = (int)(intptr_t)bullet;  // Use pointer as ID (temporary)
            msg.x = bullet->x;
            msg.y = bullet->y;
            networkManager->broadcastToAll(MessageType::BULLET_UPDATE, &msg, sizeof(msg), false);
        }
    }
}

void Game::requestRoomList() {
    if (isClient && networkManager != nullptr && networkManager->isConnected) {
        networkManager->sendRoomListRequest();
    } else if (isServer) {
        // Server: send local room list
        std::vector<RoomInfoMessage> roomList;
        for (Room* room : rooms) {
            if (room != nullptr && room->status == Room::WAITING) {
                RoomInfoMessage info;
                info.roomId = room->roomId;
                strncpy_s(info.roomName, room->roomName.c_str(), sizeof(info.roomName) - 1);
                info.roomName[sizeof(info.roomName) - 1] = '\0';
                info.playerCount = room->getPlayerCount();
                info.maxPlayers = room->maxPlayers;
                info.status = (int)room->status;
                roomList.push_back(info);
            }
        }
        handleNetworkRoomListResponse(roomList);
    }
}

void Game::displayRoomList() {
    // Room list is displayed in the display() function when menuState == ROOM_LIST
}
