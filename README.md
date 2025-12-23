# Operation Jackpot - Multiplayer Game

A 2D multiplayer battle royale game built with C++, OpenGL, and FreeGLUT.

## Game Concept

**December, 2025.**  
When history still breathes beneath the soil, a new war is born—not in blood, but in memory.

In 1971, under moonless skies and guarded rivers, Operation Jackpot was whispered only to the brave. A covert strike. A handful of fighters. The Mukti Bahini moved like shadows, seizing enemy vessels from within occupied waters. Steel met silence. Fear changed sides. It was not just a battle—it was a declaration that resistance could not be drowned.

More than five decades later, that spirit awakens again.

From the echoes of that historic triumph rises a 2D battle royale, forged in December 2025—chosen not by chance, but by meaning. Every match is chaos. Every landing is a gamble. Every survivor carries the weight of those who once fought with nothing but courage and conviction.

This is not just a game of last standing.  
It is a tribute to defiance.  
A reminder that even the smallest force, when united by purpose, can overturn the impossible.

The battlefield is unforgiving. Alliances shatter. Silence kills. Victory belongs only to those who adapt, endure, and strike at the perfect moment—just like the operation that inspired it.

Operation Jackpot is not about luck.  
It is about timing.  
It is about daring everything when history demands it.

And when the final player stands alone, it will not merely be a win.

**It will be a victory remembered.**

---

### Gameplay

Operation Jackpot is a top-down 2D battle royale where players compete in an arena. Players spawn across the map, collect weapons and equipment, and fight until one player or team remains. The game features a shrinking safe zone that forces players into combat, creating intense multiplayer matches.

**Core Features:**
- Multiple players in a single match
- Shrinking safe zone mechanics
- Weapon and loot system
- Real-time combat
- Top-down 2D perspective

**Multiplayer:** Client-server architecture and networking will be implemented in future phases.

## Architecture Overview

The game follows an object-oriented design with clear separation of concerns:

- **Game Layer**: Handles window management, rendering loop, and input processing
- **Entity Layer**: Manages player entities and game world objects
- **Rendering Layer**: Handles all OpenGL drawing operations
- **Network Layer**: Future implementation for multiplayer synchronization

The architecture is modular, making it easy to add multiplayer networking and new features without major refactoring.

## Class Structure

### Core Classes

**Game**
- Main game controller and state manager
- Handles OpenGL initialization and main game loop
- Manages menu states (NONE, JOIN_ROOM, CREATE_ROOM, PLAYING)
- Coordinates between Map and Player entities

**Player**
- Represents a player entity in the game world
- Handles movement, aiming, and rendering
- Manages player state (position, rotation, speed)
- Processes keyboard and mouse input

**Map**
- Manages the game world and environment
- Handles map rendering and boundaries
- Future: Safe zone calculations and visual effects

### Class Relationships

```
Game (1) ──→ (1) Map
  │
  └──→ (1) Player (currentPlayer)
```

- `Game` owns and manages a single `Map` instance
- `Game` maintains a reference to the current `Player`
- All classes use static methods for OpenGL callbacks

### Future Extensions

**Planned Classes:**
- `Weapon` - Weapon system with different types
- `Projectile` - Bullet and projectile physics
- `Loot` - Items and equipment spawning
- `NetworkManager` - Client-server communication
- `MatchManager` - Room and match handling

## Project Structure

```
projectOj/
├── include/              # Header files (.h)
│   ├── Game.h
│   ├── Map.h
│   └── Player.h
├── src/                  # Source files (.cpp)
│   ├── main.cpp
│   ├── Game.cpp
│   ├── Map.cpp
│   └── Player.cpp
├── assets/               # Game assets
│   └── audio/           # Audio files
│       └── gunshot.mp3
├── build/                # Build scripts and tools
│   └── run.py           # Python build script
├── bin/                  # Compiled executables
│   └── Debug/
│       └── projectOj.exe
├── obj/                  # Object files (generated)
│   └── Debug/
├── docs/                 # Documentation
├── CMakeLists.txt        # CMake build configuration
├── projectOj.cbp         # Code::Blocks project file
└── README.md            # This file
```

## Building the Project

### Using Python Build Script (Recommended)

```bash
python build/run.py
```

This will compile all source files and output to `bin/Debug/projectOj.exe`

### Using Code::Blocks 

1. Open `projectOj.cbp` in Code::Blocks
2. Build the project (F9)
3. Run the project (F10)


## Technology Stack

- **C++11** - Core programming language
- **OpenGL** - 2D graphics rendering
- **FreeGLUT** - Window management and input handling
- **Future** - Network library for multiplayer (to be determined)

## Dependencies

- **C++ Compiler** (GCC/Clang/MSVC)
- **OpenGL** - Graphics library
- **FreeGLUT** - Window and input management
- **Python 3** (optional, for build script)

### Windows
- Install MinGW-w64 or MSVC
- Download FreeGLUT and place in parent directory or system path

### Linux
```bash
sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
```

### macOS
```bash
brew install freeglut
```

## Controls

- **W/A/S/D** or **Arrow Keys** - Move player
- **Mouse** - Aim direction
- **J** - Join Room
- **C** - Create Room
- **ESC** - Return to menu

## Development

### Adding New Features

1. **New Classes**: Add header files to `include/` and source files to `src/`
2. **Assets**: Place audio files in `assets/audio/`, images in `assets/images/`, etc.
3. **Documentation**: Update this README and add docs in `docs/`

### Code Style

- Use `#pragma once` for header guards
- Follow C++11 standard
- Keep includes organized (project headers, then system headers)
- Maintain clear separation between game logic and rendering

## License


## Contributors


