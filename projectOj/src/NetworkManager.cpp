#include "NetworkManager.h"
#include "NetworkMessages.h"
#include "Game.h"
#include "Room.h"
#include "Player.h"
#include "Match.h"
#include <enet/enet.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#ifdef _WIN32
#include <WS2tcpip.h>  // Windows socket includes
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "enet64.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

NetworkManager* NetworkManager::instance = nullptr;

NetworkManager::NetworkManager()
    : mode(NetworkMode::NONE), host(nullptr), serverPeer(nullptr),
      nextPlayerId(1), localPlayerId(-1), isConnected(false),
      lastPingTime(0.0f), pingInterval(1.0f) {
    instance = this;
}

NetworkManager::~NetworkManager() {
    shutdown();
    if (instance == this) {
        instance = nullptr;
    }
}

bool NetworkManager::initialize() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet\n";
        return false;
    }
    return true;
}

void NetworkManager::shutdown() {
    if (host != nullptr) {
        if (mode == NetworkMode::SERVER) {
            stopServer();
        } else if (mode == NetworkMode::CLIENT) {
            disconnectFromServer();
        }
        enet_host_destroy(host);
        host = nullptr;
    }
    enet_deinitialize();
}

bool NetworkManager::startServer(int port) {
    if (host != nullptr) {
        stopServer();
    }
    
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    
    host = enet_host_create(&address, 32, 2, 0, 0);
    if (host == nullptr) {
        std::cerr << "Failed to create ENet server host on port " << port << "\n";
        return false;
    }
    
    mode = NetworkMode::SERVER;
    nextPlayerId = 1;
    peerToPlayerId.clear();
    playerIdToPeer.clear();
    isConnected = true;
    
    std::cout << "Server started on port " << port << "\n";
    std::cout << "Server IP: " << getLocalIP() << "\n";
    return true;
}

void NetworkManager::stopServer() {
    if (host != nullptr && mode == NetworkMode::SERVER) {
        // Disconnect all peers
        for (auto& pair : playerIdToPeer) {
            if (pair.second != nullptr) {
                enet_peer_disconnect(pair.second, 0);
            }
        }
        
        // Process events to send disconnects
        ENetEvent event;
        while (enet_host_service(host, &event, 100) > 0) {
            if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                // Peer disconnected
            }
        }
        
        peerToPlayerId.clear();
        playerIdToPeer.clear();
        mode = NetworkMode::NONE;
        isConnected = false;
        std::cout << "Server stopped\n";
    }
}

void NetworkManager::updateServer() {
    if (host == nullptr || mode != NetworkMode::SERVER) return;
    
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                int playerId = nextPlayerId++;
                peerToPlayerId[event.peer] = playerId;
                playerIdToPeer[playerId] = event.peer;
                event.peer->data = (void*)(intptr_t)playerId;
                
                // Send acceptance message
                ServerAcceptMessage acceptMsg;
                acceptMsg.playerId = playerId;
                acceptMsg.assignedId = playerId;
                sendToPeer(event.peer, MessageType::SERVER_ACCEPT, &acceptMsg, sizeof(acceptMsg), true);
                
                std::cout << "Client connected, assigned player ID: " << playerId << "\n";
                break;
            }
            
            case ENET_EVENT_TYPE_DISCONNECT: {
                int playerId = (int)(intptr_t)event.peer->data;
                peerToPlayerId.erase(event.peer);
                playerIdToPeer.erase(playerId);
                std::cout << "Client disconnected, player ID: " << playerId << "\n";
                break;
            }
            
            case ENET_EVENT_TYPE_RECEIVE: {
                // Check if this is a request that needs server handling
                if (event.packet->dataLength >= sizeof(MessageType)) {
                    MessageType type = *(MessageType*)event.packet->data;
                    void* data = event.packet->data + sizeof(MessageType);
                    size_t dataSize = event.packet->dataLength - sizeof(MessageType);
                    
                    if (type == MessageType::ROOM_LIST_REQUEST) {
                        // Handle room list request on server
                        // Get all available rooms from Game
                        std::vector<RoomInfoMessage> roomList;
                        for (Room* room : Game::rooms) {
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
                        
                        // Send room list response
                        RoomListResponseMessage header;
                        header.roomCount = roomList.size();
                        
                        // Calculate total size
                        size_t totalSize = sizeof(RoomListResponseMessage) + 
                                          roomList.size() * sizeof(RoomInfoMessage);
                        
                        // Create packet
                        MessageType msgType = MessageType::ROOM_LIST_RESPONSE;
                        ENetPacket* response = enet_packet_create(&msgType, sizeof(MessageType), 
                                                                  ENET_PACKET_FLAG_RELIABLE);
                        enet_packet_resize(response, sizeof(MessageType) + totalSize);
                        
                        // Copy header
                        memcpy(response->data + sizeof(MessageType), &header, sizeof(RoomListResponseMessage));
                        
                        // Copy room data
                        if (!roomList.empty()) {
                            memcpy(response->data + sizeof(MessageType) + sizeof(RoomListResponseMessage),
                                   roomList.data(), roomList.size() * sizeof(RoomInfoMessage));
                        }
                        
                        // Send response
                        enet_peer_send(event.peer, 0, response);
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    else if (type == MessageType::JOIN_ROOM_REQUEST) {
                        // Handle join room request on server
                        if (dataSize >= sizeof(JoinRoomRequestMessage)) {
                            JoinRoomRequestMessage* req = (JoinRoomRequestMessage*)data;
                            int playerId = (int)(intptr_t)event.peer->data;
                            
                            JoinRoomResponseMessage response;
                            response.success = false;
                            response.roomId = req->roomId;
                            
                            // Find the room
                            Room* targetRoom = nullptr;
                            for (Room* room : Game::rooms) {
                                if (room != nullptr && room->roomId == req->roomId) {
                                    targetRoom = room;
                                    break;
                                }
                            }
                            
                            if (targetRoom != nullptr && targetRoom->canJoin()) {
                                // Create or get player for this client
                                Player* player = Game::remotePlayers[playerId];
                                if (player == nullptr) {
                                    player = new Player(playerId, Game::width/2, Game::height/2);
                                    Game::remotePlayers[playerId] = player;
                                }
                                
                                // Try to add player to room
                                if (targetRoom->addPlayer(player)) {
                                    response.success = true;
                                    strncpy_s(response.roomName, targetRoom->roomName.c_str(), sizeof(response.roomName) - 1);
                                    response.roomName[sizeof(response.roomName) - 1] = '\0';
                                    response.maxPlayers = targetRoom->maxPlayers;
                                    response.playerCount = targetRoom->getPlayerCount();
                                    
                                    std::cout << "Player " << playerId << " joined room " << req->roomId << "\n";
                                } else {
                                    std::cout << "Failed to add player " << playerId << " to room " << req->roomId << "\n";
                                }
                            } else {
                                std::cout << "Room " << req->roomId << " not found or cannot be joined\n";
                            }
                            
                            // Send response
                            MessageType msgType = MessageType::JOIN_ROOM_RESPONSE;
                            ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                            enet_packet_resize(packet, sizeof(MessageType) + sizeof(JoinRoomResponseMessage));
                            memcpy(packet->data + sizeof(MessageType), &response, sizeof(JoinRoomResponseMessage));
                            enet_peer_send(event.peer, 0, packet);
                        }
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    else if (type == MessageType::CREATE_ROOM_REQUEST) {
                        // Handle create room request on server
                        if (dataSize >= sizeof(CreateRoomRequestMessage)) {
                            CreateRoomRequestMessage* req = (CreateRoomRequestMessage*)data;
                            int playerId = (int)(intptr_t)event.peer->data;
                            
                            CreateRoomResponseMessage response;
                            response.success = false;
                            
                            // Create room on server
                            Room* newRoom = Game::createRoom(std::string(req->roomName), req->maxPlayers);
                            if (newRoom != nullptr) {
                                // Create or get player for this client
                                Player* player = Game::remotePlayers[playerId];
                                if (player == nullptr) {
                                    player = new Player(playerId, Game::width/2, Game::height/2);
                                    Game::remotePlayers[playerId] = player;
                                }
                                
                                // Add player to room
                                if (newRoom->addPlayer(player)) {
                                    response.success = true;
                                    response.roomId = newRoom->roomId;
                                    strncpy_s(response.roomName, newRoom->roomName.c_str(), sizeof(response.roomName) - 1);
                                    response.roomName[sizeof(response.roomName) - 1] = '\0';
                                    response.maxPlayers = newRoom->maxPlayers;
                                    
                                    std::cout << "Player " << playerId << " created room " << newRoom->roomId << "\n";
                                }
                            }
                            
                            // Send response
                            MessageType msgType = MessageType::CREATE_ROOM_RESPONSE;
                            ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                            enet_packet_resize(packet, sizeof(MessageType) + sizeof(CreateRoomResponseMessage));
                            memcpy(packet->data + sizeof(MessageType), &response, sizeof(CreateRoomResponseMessage));
                            enet_peer_send(event.peer, 0, packet);
                        }
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    else if (type == MessageType::START_MATCH_REQUEST) {
                        // Handle start match request on server
                        if (dataSize >= sizeof(StartMatchRequestMessage)) {
                            StartMatchRequestMessage* req = (StartMatchRequestMessage*)data;
                            int playerId = (int)(intptr_t)event.peer->data;
                            
                            // Find the room the player is in
                            Room* playerRoom = nullptr;
                            std::cout << "[SERVER] Looking for player " << playerId << " in rooms...\n";
                            std::cout << "[SERVER] Total rooms: " << Game::rooms.size() << "\n";
                            
                            // Also check remotePlayers map as fallback
                            Player* requestingPlayer = nullptr;
                            auto remoteIt = Game::remotePlayers.find(playerId);
                            if (remoteIt != Game::remotePlayers.end()) {
                                requestingPlayer = remoteIt->second;
                                std::cout << "[SERVER] Found player " << playerId << " in remotePlayers map\n";
                            } else {
                                std::cout << "[SERVER] Player " << playerId << " NOT in remotePlayers map\n";
                            }
                            
                            for (Room* room : Game::rooms) {
                                if (room != nullptr) {
                                    std::cout << "[SERVER] Checking room " << room->roomId << " (status=" << (int)room->status << ") with " << room->players.size() << " players\n";
                                    for (size_t i = 0; i < room->players.size(); i++) {
                                        Player* p = room->players[i];
                                        if (p != nullptr) {
                                            std::cout << "[SERVER]   Room player[" << i << "]: ID=" << p->id << " (pointer=" << (void*)p << ")\n";
                                            if (p->id == playerId) {
                                                playerRoom = room;
                                                std::cout << "[SERVER]   ✓ Found player " << playerId << " in room " << room->roomId << "\n";
                                                break;
                                            }
                                        } else {
                                            std::cout << "[SERVER]   Room player[" << i << "]: NULL pointer!\n";
                                        }
                                    }
                                    if (playerRoom != nullptr) break;
                                }
                            }
                            
                            if (playerRoom == nullptr) {
                                std::cout << "[SERVER] ✗ Player " << playerId << " not found in any room!\n";
                                std::cout << "[SERVER] Available rooms: ";
                                for (Room* r : Game::rooms) {
                                    if (r != nullptr) {
                                        std::cout << "Room" << r->roomId << "(" << r->players.size() << " players) ";
                                    }
                                }
                                std::cout << "\n";
                            }
                            
                            if (playerRoom == nullptr) {
                                std::cout << "Cannot start match: Player " << playerId << " is not in any room\n";
                                // Send failure response
                                StartMatchResponseMessage response;
                                response.success = false;
                                response.matchId = 0;
                                
                                MessageType msgType = MessageType::START_MATCH_RESPONSE;
                                ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                                enet_packet_resize(packet, sizeof(MessageType) + sizeof(StartMatchResponseMessage));
                                memcpy(packet->data + sizeof(MessageType), &response, sizeof(StartMatchResponseMessage));
                                enet_peer_send(event.peer, 0, packet);
                            } else if (playerRoom->status == Room::IN_MATCH) {
                                // Match already started, but client wants to join - send them the match start signal
                                std::cout << "[SERVER] Match already in progress, sending join signal to player " << playerId << "\n";
                                StartMatchResponseMessage response;
                                response.success = true;
                                response.matchId = (Game::currentMatch != nullptr) ? Game::currentMatch->matchId : playerRoom->roomId;
                                
                                MessageType msgType = MessageType::START_MATCH_RESPONSE;
                                ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                                enet_packet_resize(packet, sizeof(MessageType) + sizeof(StartMatchResponseMessage));
                                memcpy(packet->data + sizeof(MessageType), &response, sizeof(StartMatchResponseMessage));
                                enet_peer_send(event.peer, 0, packet);
                            } else if (playerRoom->status != Room::WAITING) {
                                std::cout << "[SERVER] Cannot start match: Room " << playerRoom->roomId << " status is " << (int)playerRoom->status << " (not WAITING or IN_MATCH)\n";
                                // Send failure response
                                StartMatchResponseMessage response;
                                response.success = false;
                                response.matchId = 0;
                                
                                MessageType msgType = MessageType::START_MATCH_RESPONSE;
                                ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                                enet_packet_resize(packet, sizeof(MessageType) + sizeof(StartMatchResponseMessage));
                                memcpy(packet->data + sizeof(MessageType), &response, sizeof(StartMatchResponseMessage));
                                enet_peer_send(event.peer, 0, packet);
                            } else {
                                // Set server's current room before starting match
                                Game::currentRoom = playerRoom;
                                
                                // Ensure server has a player (the room creator or first player)
                                if (Game::currentPlayer == nullptr && !playerRoom->players.empty()) {
                                    // Use the first player in the room as the server's player
                                    Game::currentPlayer = playerRoom->players[0];
                                } else if (Game::currentPlayer == nullptr) {
                                    // Create a server player if none exists
                                    Game::currentPlayer = new Player(0, Game::width/2, Game::height/2);
                                }
                                
                                std::cout << "Starting match in room " << playerRoom->roomId << " with " << playerRoom->players.size() << " players\n";
                                
                                // Start the match on server
                                Game::startMatch();
                                
                                // Server also enters playing state
                                if (Game::currentMatch != nullptr && Game::currentMatch->isActive()) {
                                    Game::menuState = Game::PLAYING;
                                    std::cout << "Server entered match\n";
                                } else {
                                    std::cout << "Warning: Match started but not active\n";
                                }
                                
                                // Broadcast match start to all players in the room
                                StartMatchResponseMessage response;
                                response.success = true;
                                response.matchId = (Game::currentMatch != nullptr) ? Game::currentMatch->matchId : 0;
                                
                                MessageType msgType = MessageType::START_MATCH_RESPONSE;
                                
                                // Send to all players in the room
                                int sentCount = 0;
                                for (Player* p : playerRoom->players) {
                                    if (p != nullptr) {
                                        auto it = Game::networkManager->playerIdToPeer.find(p->id);
                                        if (it != Game::networkManager->playerIdToPeer.end() && it->second != nullptr) {
                                            ENetPacket* packet = enet_packet_create(&msgType, sizeof(MessageType), ENET_PACKET_FLAG_RELIABLE);
                                            enet_packet_resize(packet, sizeof(MessageType) + sizeof(StartMatchResponseMessage));
                                            memcpy(packet->data + sizeof(MessageType), &response, sizeof(StartMatchResponseMessage));
                                            enet_peer_send(it->second, 0, packet);
                                            sentCount++;
                                        } else {
                                            std::cout << "Warning: No peer found for player " << p->id << "\n";
                                        }
                                    }
                                }
                                
                                std::cout << "Match started in room " << playerRoom->roomId << ", sent to " << sentCount << " players\n";
                            }
                        }
                        enet_packet_destroy(event.packet);
                        break;
                    }
                }
                
                handleMessage(event.peer, event.packet);
                enet_packet_destroy(event.packet);
                break;
            }
            
            default:
                break;
        }
    }
}

void NetworkManager::broadcastToAll(MessageType type, const void* data, size_t dataSize, bool reliable) {
    if (host == nullptr || mode != NetworkMode::SERVER) return;
    
    ENetPacket* packet = enet_packet_create(&type, sizeof(MessageType), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    enet_packet_resize(packet, sizeof(MessageType) + dataSize);
    memcpy(packet->data + sizeof(MessageType), data, dataSize);
    
    enet_host_broadcast(host, 0, packet);
}

void NetworkManager::sendToPeer(ENetPeer* peer, MessageType type, const void* data, size_t dataSize, bool reliable) {
    if (host == nullptr || peer == nullptr) return;
    
    ENetPacket* packet = enet_packet_create(&type, sizeof(MessageType), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    enet_packet_resize(packet, sizeof(MessageType) + dataSize);
    memcpy(packet->data + sizeof(MessageType), data, dataSize);
    
    enet_peer_send(peer, 0, packet);
}

bool NetworkManager::connectToServer(const std::string& hostname, int port) {
    if (host != nullptr) {
        disconnectFromServer();
    }
    
    host = enet_host_create(nullptr, 1, 2, 0, 0);
    if (host == nullptr) {
        std::cerr << "Failed to create ENet client host\n";
        return false;
    }
    
    ENetAddress address;
    enet_address_set_host(&address, hostname.c_str());
    address.port = port;
    
    serverPeer = enet_host_connect(host, &address, 2, 0);
    if (serverPeer == nullptr) {
        std::cerr << "Failed to connect to server\n";
        enet_host_destroy(host);
        host = nullptr;
        return false;
    }
    
    mode = NetworkMode::CLIENT;
    isConnected = false;  // Will be set to true when we receive SERVER_ACCEPT
    
    std::cout << "Connecting to server at " << hostname << ":" << port << "...\n";
    return true;
}

void NetworkManager::disconnectFromServer() {
    if (serverPeer != nullptr) {
        enet_peer_disconnect(serverPeer, 0);
        
        ENetEvent event;
        while (enet_host_service(host, &event, 3000) > 0) {
            if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                break;
            }
        }
        
        serverPeer = nullptr;
    }
    
    if (host != nullptr) {
        enet_host_destroy(host);
        host = nullptr;
    }
    
    mode = NetworkMode::NONE;
    isConnected = false;
    localPlayerId = -1;
}

void NetworkManager::updateClient() {
    if (host == nullptr || mode != NetworkMode::CLIENT) return;
    
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                std::cout << "Connected to server\n";
                break;
            }
            
            case ENET_EVENT_TYPE_DISCONNECT: {
                std::cout << "Disconnected from server\n";
                isConnected = false;
                localPlayerId = -1;
                break;
            }
            
            case ENET_EVENT_TYPE_RECEIVE: {
                handleMessage(event.peer, event.packet);
                enet_packet_destroy(event.packet);
                break;
            }
            
            default:
                break;
        }
    }
}

void NetworkManager::update() {
    if (mode == NetworkMode::SERVER) {
        updateServer();
    } else if (mode == NetworkMode::CLIENT) {
        updateClient();
    }
}

void NetworkManager::sendToServer(MessageType type, const void* data, size_t dataSize, bool reliable) {
    if (host == nullptr || serverPeer == nullptr || mode != NetworkMode::CLIENT) return;
    
    ENetPacket* packet = enet_packet_create(&type, sizeof(MessageType), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    enet_packet_resize(packet, sizeof(MessageType) + dataSize);
    memcpy(packet->data + sizeof(MessageType), data, dataSize);
    
    enet_peer_send(serverPeer, 0, packet);
}

void NetworkManager::sendPlayerInput(bool w, bool s, bool a, bool d, float mouseX, float mouseY, bool shoot) {
    PlayerInputMessage msg;
    msg.keys = 0;
    if (w) msg.keys |= 0x01;
    if (s) msg.keys |= 0x02;
    if (a) msg.keys |= 0x04;
    if (d) msg.keys |= 0x08;
    if (shoot) msg.keys |= 0x10;
    msg.mouseX = mouseX;
    msg.mouseY = mouseY;
    
    sendToServer(MessageType::PLAYER_INPUT, &msg, sizeof(msg), false);  // Unreliable for frequent updates
}

void NetworkManager::sendCreateRoomRequest(const std::string& roomName, int maxPlayers) {
    CreateRoomRequestMessage msg;
    strncpy_s(msg.roomName, roomName.c_str(), sizeof(msg.roomName) - 1);
    msg.roomName[sizeof(msg.roomName) - 1] = '\0';
    msg.maxPlayers = maxPlayers;
    
    sendToServer(MessageType::CREATE_ROOM_REQUEST, &msg, sizeof(msg), true);
}

void NetworkManager::sendJoinRoomRequest(int roomId) {
    JoinRoomRequestMessage msg;
    msg.roomId = roomId;
    
    sendToServer(MessageType::JOIN_ROOM_REQUEST, &msg, sizeof(msg), true);
}

void NetworkManager::sendRoomListRequest() {
    // Empty message, just the type
    sendToServer(MessageType::ROOM_LIST_REQUEST, nullptr, 0, true);
}

void NetworkManager::sendStartMatchRequest() {
    StartMatchRequestMessage msg;
    msg.roomId = 0;  // Will be set by server based on current room
    sendToServer(MessageType::START_MATCH_REQUEST, &msg, sizeof(msg), true);
}

void NetworkManager::handleMessage(ENetPeer* peer, ENetPacket* packet) {
    if (packet->dataLength < sizeof(MessageType)) return;
    
    MessageType type = *(MessageType*)packet->data;
    void* data = packet->data + sizeof(MessageType);
    size_t dataSize = packet->dataLength - sizeof(MessageType);
    
    switch (type) {
        case MessageType::SERVER_ACCEPT: {
            if (dataSize >= sizeof(ServerAcceptMessage)) {
                ServerAcceptMessage* msg = (ServerAcceptMessage*)data;
                localPlayerId = msg->playerId;
                isConnected = true;
                std::cout << "Server accepted connection, assigned player ID: " << localPlayerId << "\n";
            }
            break;
        }
        
        case MessageType::PLAYER_STATE: {
            if (dataSize >= sizeof(PlayerStateMessage)) {
                PlayerStateMessage* msg = (PlayerStateMessage*)data;
                if (onPlayerStateUpdate) {
                    onPlayerStateUpdate(msg->playerId, msg->x, msg->y, msg->angle);
                }
            }
            break;
        }
        
        case MessageType::BULLET_SPAWN: {
            if (dataSize >= sizeof(BulletSpawnMessage)) {
                BulletSpawnMessage* msg = (BulletSpawnMessage*)data;
                if (onBulletSpawn) {
                    onBulletSpawn(msg->bulletId, msg->x, msg->y, msg->vx, msg->vy, msg->ownerId);
                }
            }
            break;
        }
        
        case MessageType::BULLET_UPDATE: {
            if (dataSize >= sizeof(BulletUpdateMessage)) {
                BulletUpdateMessage* msg = (BulletUpdateMessage*)data;
                if (onBulletUpdate) {
                    onBulletUpdate(msg->bulletId, msg->x, msg->y);
                }
            }
            break;
        }
        
        case MessageType::BULLET_REMOVE: {
            if (dataSize >= sizeof(BulletRemoveMessage)) {
                BulletRemoveMessage* msg = (BulletRemoveMessage*)data;
                if (onBulletRemove) {
                    onBulletRemove(msg->bulletId);
                }
            }
            break;
        }
        
        case MessageType::CREATE_ROOM_RESPONSE: {
            if (dataSize >= sizeof(CreateRoomResponseMessage)) {
                CreateRoomResponseMessage* msg = (CreateRoomResponseMessage*)data;
                if (onCreateRoomResponse) {
                    onCreateRoomResponse(msg->success, msg->roomId);
                }
            }
            break;
        }
        
        case MessageType::JOIN_ROOM_RESPONSE: {
            if (dataSize >= sizeof(JoinRoomResponseMessage)) {
                JoinRoomResponseMessage* msg = (JoinRoomResponseMessage*)data;
                if (onJoinRoomResponse) {
                    onJoinRoomResponse(msg->success, msg->roomId);
                }
            }
            break;
        }
        
        case MessageType::ROOM_UPDATE: {
            if (dataSize >= sizeof(RoomInfoMessage)) {
                RoomInfoMessage* msg = (RoomInfoMessage*)data;
                if (onRoomUpdate) {
                    onRoomUpdate(msg->roomId, std::string(msg->roomName), msg->playerCount, msg->maxPlayers);
                }
            }
            break;
        }
        
        case MessageType::ROOM_LIST_RESPONSE: {
            if (dataSize >= sizeof(RoomListResponseMessage)) {
                RoomListResponseMessage* header = (RoomListResponseMessage*)data;
                std::vector<RoomInfoMessage> roomList;
                
                // Read room count
                int roomCount = header->roomCount;
                size_t offset = sizeof(RoomListResponseMessage);
                
                // Read each room info
                for (int i = 0; i < roomCount && offset + sizeof(RoomInfoMessage) <= dataSize; i++) {
                    RoomInfoMessage* roomInfo = (RoomInfoMessage*)((char*)data + offset);
                    roomList.push_back(*roomInfo);
                    offset += sizeof(RoomInfoMessage);
                }
                
                if (onRoomListResponse) {
                    onRoomListResponse(roomList);
                }
            }
            break;
        }
        
        case MessageType::START_MATCH_RESPONSE: {
            if (dataSize >= sizeof(StartMatchResponseMessage)) {
                StartMatchResponseMessage* msg = (StartMatchResponseMessage*)data;
                if (msg->success && onMatchStart) {
                    onMatchStart();
                } else {
                    std::cout << "Failed to start match\n";
                }
            }
            break;
        }
        
        case MessageType::PLAYER_INPUT: {
            // Server receives player input from clients
            if (mode == NetworkMode::SERVER && dataSize >= sizeof(PlayerInputMessage)) {
                PlayerInputMessage* msg = (PlayerInputMessage*)data;
                int playerId = (int)(intptr_t)peer->data;
                
                if (onPlayerInput) {
                    bool w = (msg->keys & 0x01) != 0;
                    bool s = (msg->keys & 0x02) != 0;
                    bool a = (msg->keys & 0x04) != 0;
                    bool d = (msg->keys & 0x08) != 0;
                    bool shoot = (msg->keys & 0x10) != 0;
                    onPlayerInput(playerId, w, s, a, d, msg->mouseX, msg->mouseY, shoot);
                }
            }
            break;
        }
        
        default:
            break;
    }
}

std::string NetworkManager::getLocalIP() {
    std::vector<std::string> ips = getLocalIPs();
    if (!ips.empty()) {
        return ips[0];
    }
    return "127.0.0.1";
}

std::vector<std::string> NetworkManager::getLocalIPs() {
    std::vector<std::string> ips;
    
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return ips;
    }
#endif
    
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        struct addrinfo* result = nullptr;
        if (getaddrinfo(hostname, nullptr, &hints, &result) == 0) {
            for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
                if (ptr->ai_family == AF_INET) {
                    char ip[INET_ADDRSTRLEN];
                    struct sockaddr_in* sockaddr = (struct sockaddr_in*)ptr->ai_addr;
                    if (inet_ntop(AF_INET, &sockaddr->sin_addr, ip, INET_ADDRSTRLEN) != nullptr) {
                        std::string ipStr(ip);
                        if (ipStr != "127.0.0.1") {
                            ips.push_back(ipStr);
                        }
                    }
                }
            }
            freeaddrinfo(result);
        }
    }
    
    if (ips.empty()) {
        ips.push_back("127.0.0.1");
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return ips;
}

