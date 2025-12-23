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

int Game::width = 0;
int Game::height = 0;
Game::MenuState Game::menuState = Game::NONE;
Player* Game::currentPlayer = nullptr;
Map* Game::gameMap = nullptr;
Room* Game::currentRoom = nullptr;
Match* Game::currentMatch = nullptr;
std::vector<Room*> Game::rooms;
std::vector<Bullet*> Game::bullets;
float Game::mouseX = 0.0f;
float Game::mouseY = 0.0f;

Game::Game(int w, int h, int argc, char** argv) {
    width = w;
    height = h;
    runOpenGl(argc, argv);
}

void Game::runOpenGl(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("2D Battle Royale");

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
        drawText(300, 350, "J - Join Room");
        drawText(300, 300, "C - Create Room");
    }
    else if (menuState == JOIN_ROOM) {
        drawText(280, 325, "Joining Room...");
    }
    else if (menuState == IN_ROOM) {
        if (currentRoom != nullptr) {
            drawText(280, 400, "Room Created!");
            drawText(250, 350, "Players: " + std::to_string(currentRoom->getPlayerCount()) + 
                      "/" + std::to_string(currentRoom->maxPlayers));
            drawText(250, 300, "S - Start Match");
            drawText(250, 250, "ESC - Back to Menu");
        }
    }
    else if (menuState == PLAYING) {
        if (gameMap != nullptr) {
            gameMap->render();
        }
        if (currentPlayer != nullptr) {
            currentPlayer->render();
        }
        // Render all bullets
        for (Bullet* bullet : bullets) {
            if (bullet != nullptr) {
                bullet->render();
            }
        }
        // Draw crosshair cursor
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
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->updateMovement();
        }
        updateBullets();
        cleanupBullets();
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
            startMatch();
            if (currentMatch != nullptr && currentMatch->isActive()) {
                menuState = PLAYING;
                std::cout << "Match started!\n";
            }
        }
        else if (key == 27) {
            menuState = NONE;
            std::cout << "Back to menu\n";
        }
    }
    else {
        if (key == 'j' || key == 'J') {
            menuState = JOIN_ROOM;
            std::cout << "Join Room selected\n";
        }
        else if (key == 'c' || key == 'C') {
            // Create a room
            if (currentPlayer == nullptr) {
                currentPlayer = new Player(1, width/2, height/2);
            }
            Room* newRoom = createRoom("Room " + std::to_string(rooms.size() + 1), 4);
            // Add the current player to the room
            if (newRoom != nullptr && currentPlayer != nullptr) {
                newRoom->addPlayer(currentPlayer);
            }
            menuState = IN_ROOM;
            std::cout << "Room created! Press S to start match.\n";
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
    // Always update mouse position
    mouseX = x;
    mouseY = height - y;  // Flip Y coordinate (GLUT has origin at top-left, OpenGL at bottom-left)
    
    if (menuState == PLAYING) {
        if (currentPlayer != nullptr) {
            currentPlayer->updateAim(mouseX, mouseY);
            glutPostRedisplay();
        }
    }
}

void Game::mouseClick(int button, int state, int x, int y) {
    if (menuState == PLAYING && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentPlayer != nullptr) {
            // Convert mouse coordinates
            float mouseX = x;
            float mouseY = height - y;
            
            // Get bullet spawn position
            float spawnX, spawnY;
            currentPlayer->getBulletSpawnPosition(spawnX, spawnY);
            
            // Create new bullet
            Bullet* newBullet = new Bullet(spawnX, spawnY, currentPlayer->angle, currentPlayer->id);
            bullets.push_back(newBullet);
            
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
    float size = 15.0f;  // Size of crosshair lines
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
    for (Bullet* bullet : bullets) {
        if (bullet != nullptr) {
            bullet->update();
            // Deactivate bullets that go out of bounds
            if (bullet->isOutOfBounds(width, height)) {
                bullet->active = false;
            }
        }
    }
}

void Game::cleanupBullets() {
    // Remove inactive bullets
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
    if (currentRoom != nullptr && currentRoom->status == Room::WAITING) {
        // Initialize game map if not already created
        if (gameMap == nullptr) {
            gameMap = new Map(width, height);
        }
        
        // Ensure player exists
        if (currentPlayer == nullptr) {
            currentPlayer = new Player(1, width/2, height/2);
        }
        
        currentRoom->setStatus(Room::STARTING);
        int matchId = rooms.size(); // Simple ID generation
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
    }
}
