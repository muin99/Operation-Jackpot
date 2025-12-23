# Project Structure Documentation

## Directory Layout

### `/include/`
Contains all C++ header files (.h). These files define the interfaces and class declarations.

**Files:**
- `Game.h` - Main game class and menu state management
- `Map.h` - Map rendering class
- `Player.h` - Player entity class

### `/src/`
Contains all C++ source files (.cpp). These files implement the functionality declared in headers.

**Files:**
- `main.cpp` - Application entry point
- `Game.cpp` - Game logic implementation
- `Map.cpp` - Map rendering implementation
- `Player.cpp` - Player movement and rendering

### `/assets/`
Contains all game assets (non-code resources).

**Subdirectories:**
- `/assets/audio/` - Sound effects and music files
  - `gunshot.mp3` - Gunshot sound effect

**Future additions:**
- `/assets/images/` - Textures and sprites
- `/assets/fonts/` - Font files
- `/assets/models/` - 3D models (if applicable)

### `/build/`
Contains build scripts and build-related tools.

**Files:**
- `run.py` - Python build script for quick compilation

**Future additions:**
- Makefile
- Build configuration files

### `/bin/`
Contains compiled executables (generated, not committed to version control).

**Structure:**
- `/bin/Debug/` - Debug builds
- `/bin/Release/` - Release builds (future)

### `/obj/`
Contains compiled object files (generated, not committed to version control).

**Structure:**
- `/obj/Debug/` - Debug object files
- `/obj/Release/` - Release object files (future)

### `/docs/`
Contains project documentation.

**Files:**
- `BUILD.md` - Build instructions
- `STRUCTURE.md` - This file

## Build System Files

### Root Level
- `CMakeLists.txt` - CMake build configuration (modern, cross-platform)
- `projectOj.cbp` - Code::Blocks project file
- `.gitignore` - Git ignore patterns
- `README.md` - Main project documentation

## Generic Practices

1. **Headers in `/include/`**: All `.h` files go here
2. **Sources in `/src/`**: All `.cpp` files go here
3. **Assets organized by type**: Use subdirectories in `/assets/` for different asset types
4. **Build outputs**: Never commit `/bin/` or `/obj/` directories
5. **Documentation**: Keep docs updated in `/docs/`

## Adding New Files

### Adding a New Class
1. Create header in `/include/ClassName.h`
2. Create implementation in `/src/ClassName.cpp`
3. Update `CMakeLists.txt` and `projectOj.cbp` if needed

### Adding Assets
1. Place in appropriate `/assets/` subdirectory
2. Create subdirectory if new asset type
3. Update documentation if needed

### Adding Documentation
1. Place markdown files in `/docs/`
2. Update `README.md` with links if needed

