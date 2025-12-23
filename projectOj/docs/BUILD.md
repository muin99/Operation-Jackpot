# Build Instructions

## Prerequisites

- C++11 compatible compiler (GCC 4.8+, Clang 3.3+, MSVC 2013+)
- OpenGL development libraries
- FreeGLUT library
- CMake 3.10+ (optional, for CMake builds)

## Build Methods

### Method 1: Code::Blocks

1. Open `projectOj.cbp`
2. Configure compiler paths if needed
3. Build → Build (F9)
4. Run → Run (F10)

### Method 2: Python Script

```bash
python build/run.py
```

### Method 3: Manual Compilation

```bash
g++ -Iinclude src/*.cpp -o bin/Debug/projectOj -lfreeglut -lopengl32 -lglu32
```

## Troubleshooting

### FreeGLUT not found
- Windows: Ensure FreeGLUT DLLs are in PATH or same directory as executable
- Linux: Install `freeglut3-dev` package
- macOS: Install via Homebrew: `brew install freeglut`

### OpenGL errors
- Ensure graphics drivers are up to date
- Check that OpenGL is properly installed on your system

### Include path errors
- Verify that `include/` directory contains all header files
- Check compiler include path settings

