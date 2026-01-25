#include "Game.h"
#include "Player.h"
#include "Map.h"
#include "Room.h"
#include "Match.h"
#include "Bullet.h"
#include "Sound.h"
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <cmath>

int Game::width = 0;
int Game::height = 0;
Game::MenuState Game::menuState = Game::NONE;
Player* Game::currentPlayer = nullptr;
Map* Game::gameMap = nullptr;
Room* Game::currentRoom = nullptr;
Match* Game::currentMatch = nullptr;
std::vector<Room*> Game::rooms;
std::vector<Bullet*> Game::bullets;
std::vector<Player*> Game::allPlayers;
float Game::mouseX = 0.0f;
float Game::mouseY = 0.0f;
float Game::cameraX = 0.0f;
float Game::cameraY = 0.0f;

Game::Game(int w, int h, int argc, char** argv) {
    width = w;
    height = h;
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
    glutTimerFunc(16, timer, 0);
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
        drawText(300, 450, "C - Create Room");
        drawText(300, 400, "J - Join Room");
        drawText(300, 350, "ESC - Exit");
    }
    else if (menuState == ROOM_LIST) {
        drawText(280, 500, "Available Rooms:");
        drawText(250, 470, "Press number key to join (1-9)");
        drawText(250, 440, "R - Refresh list");
        drawText(250, 410, "ESC - Back to menu");
        
        float yPos = 380.0f;
        int displayIndex = 0;
        for (size_t i = 0; i < rooms.size() && displayIndex < 9; i++) {
            Room* room = rooms[i];
            if (room != nullptr && room->canJoin()) {
                std::string roomText = std::to_string(displayIndex + 1) + ". " + room->roomName + 
                                       " (" + std::to_string(room->getPlayerCount()) + "/" + 
                                       std::to_string(room->maxPlayers) + ")";
                drawText(250, yPos, roomText);
                yPos -= 30.0f;
                displayIndex++;
            }
        }
        
        if (displayIndex == 0) {
            drawText(250, 350, "No rooms available");
            drawText(250, 320, "Press C to create a room");
        }
    }
    else if (menuState == IN_ROOM) {
        if (currentRoom != nullptr) {
            drawText(280, 450, "Room: " + currentRoom->roomName);
            drawText(250, 420, "Players: " + std::to_string(currentRoom->getPlayerCount()) + 
                      "/" + std::to_string(currentRoom->maxPlayers));
            
            float yPos = 380.0f;
            drawText(250, yPos, "Players in room:");
            yPos -= 25.0f;
            for (size_t i = 0; i < currentRoom->players.size() && i < 8; i++) {
                Player* p = currentRoom->players[i];
                if (p != nullptr) {
                    std::string playerText = "  Player " + std::to_string(p->id);
                    if (p == currentPlayer) {
                        playerText += " (You)";
                    }
                    drawText(250, yPos, playerText);
                    yPos -= 20.0f;
                }
            }
            
            if (currentRoom->getPlayerCount() > 0) {
                drawText(250, yPos - 10, "S - Start Match");
            }
            drawText(250, yPos - 40, "ESC - Leave Room");
        }
    }
    else if (menuState == PLAYING) {
        if (currentPlayer != nullptr && currentPlayer->isAlive) {
            cameraX = currentPlayer->x - width / 2.0f;
            cameraY = currentPlayer->y - height / 2.0f;
        }
        
        glPushMatrix();
        glTranslatef(-cameraX, -cameraY, 0.0f);
        
        if (gameMap != nullptr) {
            gameMap->render();
        }
        
        for (Bullet* bullet : bullets) {
            if (bullet != nullptr && bullet->active) {
                bullet->render();
            }
        }
        
        for (Player* player : allPlayers) {
            if (player != nullptr && player->isAlive && player != currentPlayer) {
                glPushMatrix();
                glTranslatef(player->x, player->y, 0.0f);
                glRotatef(player->angle * 180.0f / 3.14159f, 0.0f, 0.0f, 1.0f);
                
                glColor3f(1.0f, 0.0f, 0.0f);
                glBegin(GL_TRIANGLES);
                glVertex2f(0, player->size / 2);
                glVertex2f(-player->size / 2, -player->size / 2);
                glVertex2f(player->size / 2, -player->size / 2);
                glEnd();
                
                glPopMatrix();
            }
        }
        
        glPopMatrix();
        
        if (currentPlayer != nullptr && currentPlayer->isAlive) {
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
        
        drawCrosshair(mouseX, mouseY);
        
        int aliveCount = 0;
        for (Player* p : allPlayers) {
            if (p != nullptr && p->isAlive) {
                aliveCount++;
            }
        }
        drawText(10, height - 30, "Alive: " + std::to_string(aliveCount));
    }
    else if (menuState == MATCH_ENDED) {
        Player* winner = nullptr;
        for (Player* p : allPlayers) {
            if (p != nullptr && p->isAlive) {
                winner = p;
                break;
            }
        }
        
        if (winner != nullptr) {
            drawText(300, 400, "Match Ended!");
            if (winner == currentPlayer) {
                drawText(280, 350, "You Win!");
            } else {
                drawText(280, 350, "Player " + std::to_string(winner->id) + " Wins!");
            }
        } else {
            drawText(300, 400, "Match Ended!");
            drawText(280, 350, "No Winner");
        }
        
        drawText(250, 300, "Press ESC to return to menu");
    }

    glutSwapBuffers();
}

void Game::idle() {
    if (menuState == PLAYING) {
        glutPostRedisplay();
    }
}

void Game::timer(int value) {
    if (menuState == PLAYING) {
        // Create collision check lambda (gameMap is static, so we can access it directly)
        auto checkCollision = [](float x, float y, float radius) -> bool {
            if (Game::gameMap == nullptr) return false;
            return Game::gameMap->checkCollision(x, y, radius);
        };
        
        for (Player* player : allPlayers) {
            if (player != nullptr && player->isAlive) {
                player->updateMovementWithCollision(checkCollision);
            }
        }
        
        updateBullets();
        checkBulletCollisions();
        cleanupBullets();
        checkWinCondition();
        
        glutPostRedisplay();
    }
    glutTimerFunc(16, timer, 0);
}

void Game::keyPressed(unsigned char key, int, int) {
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr && currentPlayer->isAlive) {
            currentPlayer->handleKey(key, true);
        }
        if (key == 27) {
            menuState = NONE;
            glutSetCursor(GLUT_CURSOR_INHERIT);
            for (Bullet* bullet : bullets) {
                if (bullet != nullptr) {
                    delete bullet;
                }
            }
            bullets.clear();
        }
    }
    else if (menuState == MATCH_ENDED) {
        if (key == 27) {
            menuState = NONE;
            if (currentMatch != nullptr) {
                delete currentMatch;
                currentMatch = nullptr;
            }
            for (Player* p : allPlayers) {
                if (p != nullptr && p != currentPlayer) {
                    delete p;
                }
            }
            allPlayers.clear();
            if (currentPlayer != nullptr) {
                allPlayers.push_back(currentPlayer);
            }
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
            startMatch();
            if (currentMatch != nullptr && currentMatch->isActive()) {
                menuState = PLAYING;
                std::cout << "Match started!\n";
            }
        }
        else if (key == 27) {
            currentRoom = nullptr;
            menuState = NONE;
            std::cout << "Left room\n";
        }
    }
    else if (menuState == ROOM_LIST) {
        if (key >= '1' && key <= '9') {
            int roomIndex = key - '1';
            int displayIndex = 0;
            for (size_t i = 0; i < rooms.size(); i++) {
                Room* room = rooms[i];
                if (room != nullptr && room->canJoin()) {
                    if (displayIndex == roomIndex) {
                        if (joinRoom(room->roomId)) {
                            menuState = IN_ROOM;
                        }
                        break;
                    }
                    displayIndex++;
                }
            }
        }
        else if (key == 'r' || key == 'R') {
            requestRoomList();
        }
        else if (key == 27) {
            menuState = NONE;
        }
    }
    else {
        if (key == 'j' || key == 'J') {
            requestRoomList();
            menuState = ROOM_LIST;
        }
        else if (key == 'c' || key == 'C') {
            if (currentPlayer == nullptr) {
                currentPlayer = new Player(1, width/2, height/2);
                allPlayers.push_back(currentPlayer);
            }
            
            Room* newRoom = createRoom("Room " + std::to_string(rooms.size() + 1), 4);
            if (newRoom != nullptr && currentPlayer != nullptr) {
                newRoom->addPlayer(currentPlayer);
                
                // Add 2 simple target players for local gameplay (they won't move)
                Player* target1 = new Player(2, width/2, height/2);
                Player* target2 = new Player(3, width/2, height/2);
                newRoom->addPlayer(target1);
                newRoom->addPlayer(target2);
                allPlayers.push_back(target1);
                allPlayers.push_back(target2);
            }
            menuState = IN_ROOM;
            std::cout << "Room created with " << newRoom->getPlayerCount() << " players! Press S to start match.\n";
        }
        else if (key == 27) {
            menuState = NONE;
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
        if (currentPlayer != nullptr && currentPlayer->isAlive) {
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
    mouseX = x;
    mouseY = height - y;
    
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr && currentPlayer->isAlive) {
            float worldMouseX = currentPlayer->x + (mouseX - width / 2.0f);
            float worldMouseY = currentPlayer->y + (mouseY - height / 2.0f);
            currentPlayer->updateAim(worldMouseX, worldMouseY);
            glutPostRedisplay();
        }
    }
}

void Game::mouseClick(int button, int state, int x, int y) {
    if (menuState == PLAYING && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentPlayer != nullptr && currentPlayer->isAlive) {
            float screenMouseX = x;
            float screenMouseY = height - y;
            float worldMouseX = currentPlayer->x + (screenMouseX - width / 2.0f);
            float worldMouseY = currentPlayer->y + (screenMouseY - height / 2.0f);
            
            float spawnX, spawnY;
            currentPlayer->getBulletSpawnPosition(spawnX, spawnY);
            Bullet* newBullet = new Bullet(spawnX, spawnY, currentPlayer->angle, currentPlayer->id);
            bullets.push_back(newBullet);
            
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
    float size = 8.0f;
    float thickness = 2.0f;
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(thickness);
    
    glBegin(GL_LINES);
    glVertex2f(x - size, y);
    glVertex2f(x + size, y);
    glVertex2f(x, y - size);
    glVertex2f(x, y + size);
    glEnd();
    
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
    
    glLineWidth(1.0f);
}

void Game::updateBullets() {
    for (Bullet* bullet : bullets) {
        if (bullet != nullptr && bullet->active) {
            // Store old position
            float oldX = bullet->x;
            float oldY = bullet->y;
            
            // Update bullet position
            bullet->update();
            
            // Check collision with map obstacles
            if (gameMap != nullptr && gameMap->checkCollision(bullet->x, bullet->y, bullet->size)) {
                bullet->active = false;
                bullet->x = oldX;  // Revert position
                bullet->y = oldY;
            }
            
            // Check bounds
            if (bullet->isOutOfBounds(width, height)) {
                bullet->active = false;
            }
        }
    }
}

void Game::checkBulletCollisions() {
    for (Bullet* bullet : bullets) {
        if (bullet == nullptr || !bullet->active) continue;
        
        for (Player* player : allPlayers) {
            if (player == nullptr || !player->isAlive) continue;
            if (player->id == bullet->ownerId) continue;
            
            float dx = bullet->x - player->x;
            float dy = bullet->y - player->y;
            float distance = sqrt(dx * dx + dy * dy);
            
            if (distance < player->size / 2.0f + bullet->size) {
                player->eliminate();
                bullet->active = false;
                std::cout << "Player " << player->id << " eliminated by Player " << bullet->ownerId << "!\n";
                break;
            }
        }
    }
}

void Game::checkWinCondition() {
    if (menuState != PLAYING) return;
    
    int aliveCount = 0;
    Player* lastAlive = nullptr;
    
    for (Player* player : allPlayers) {
        if (player != nullptr && player->isAlive) {
            aliveCount++;
            lastAlive = player;
        }
    }
    
    if (aliveCount <= 1 && allPlayers.size() > 1) {
        menuState = MATCH_ENDED;
        std::cout << "Match ended! ";
        if (lastAlive != nullptr) {
            std::cout << "Player " << lastAlive->id << " wins!\n";
        }
    }
}

void Game::cleanupBullets() {
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
                allPlayers.push_back(currentPlayer);
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
        std::cout << "Cannot start match: room status is not WAITING\n";
        return;
    }
    
    if (gameMap == nullptr) {
        gameMap = new Map(width, height);
    }
    
    if (currentPlayer == nullptr) {
        if (!currentRoom->players.empty()) {
            currentPlayer = currentRoom->players[0];
        } else {
            currentPlayer = new Player(1, width/2, height/2);
            currentRoom->addPlayer(currentPlayer);
        }
    }
    
    allPlayers.clear();
    for (Player* p : currentRoom->players) {
        if (p != nullptr) {
            p->isAlive = true;
            allPlayers.push_back(p);
        }
    }
    
    // Spawn players in valid positions (avoiding obstacles)
    float spawnRadius = std::min(width, height) * 0.35f;
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float playerRadius = 15.0f;  // Player collision radius
    
    for (size_t i = 0; i < allPlayers.size(); i++) {
        Player* p = allPlayers[i];
        if (p != nullptr) {
            // Try to find a valid spawn position
            bool foundValidPosition = false;
            int attempts = 0;
            float angle = (2.0f * 3.14159f * i) / allPlayers.size();
            
            // Try multiple angles around the circle
            for (int attempt = 0; attempt < 20 && !foundValidPosition; attempt++) {
                float testAngle = angle + (attempt * 0.1f);
                float testX = centerX + cos(testAngle) * spawnRadius;
                float testY = centerY + sin(testAngle) * spawnRadius;
                
                // Try different radii if needed
                for (float radiusMult = 1.0f; radiusMult >= 0.5f && !foundValidPosition; radiusMult -= 0.1f) {
                    float testX2 = centerX + cos(testAngle) * spawnRadius * radiusMult;
                    float testY2 = centerY + sin(testAngle) * spawnRadius * radiusMult;
                    
                    if (gameMap->isValidSpawnPosition(testX2, testY2, playerRadius)) {
                        p->x = testX2;
                        p->y = testY2;
                        foundValidPosition = true;
                        break;
                    }
                }
            }
            
            // Fallback: spawn at corners if circle positions fail
            if (!foundValidPosition) {
                float corners[4][2] = {
                    {width * 0.15f, height * 0.15f},
                    {width * 0.85f, height * 0.15f},
                    {width * 0.15f, height * 0.85f},
                    {width * 0.85f, height * 0.85f}
                };
                int cornerIndex = i % 4;
                p->x = corners[cornerIndex][0];
                p->y = corners[cornerIndex][1];
                
                // If corner also fails, just use a safe default
                if (!gameMap->isValidSpawnPosition(p->x, p->y, playerRadius)) {
                    p->x = width * 0.1f + (i * 50.0f);
                    p->y = height * 0.1f + (i * 50.0f);
                }
            }
        }
    }
    
    currentRoom->setStatus(Room::STARTING);
    int matchId = currentRoom->roomId;
    currentMatch = new Match(matchId, currentRoom);
    currentMatch->gameMap = gameMap;
    currentMatch->start();
    currentRoom->setStatus(Room::IN_MATCH);
    
    mouseX = width / 2.0f;
    mouseY = height / 2.0f;
    
    for (Bullet* bullet : bullets) {
        if (bullet != nullptr) {
            delete bullet;
        }
    }
    bullets.clear();
    
    glutSetCursor(GLUT_CURSOR_NONE);
    
    std::cout << "Match started! Room " << currentRoom->roomId << " with " << currentRoom->getPlayerCount() << " players\n";
}

void Game::requestRoomList() {
    // Room list is just the local rooms vector
}

