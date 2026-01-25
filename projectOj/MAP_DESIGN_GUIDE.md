# Map Design Guide

This guide explains how to customize the map layout in `Map.cpp`.

## Quick Start

Edit the `initializeObstacles()` function in `Map.cpp` to customize your map design.

## Available Obstacle Types

### 1. Rectangles
```cpp
addRectangle(x, y, width, height);
// Example:
addRectangle(100, 100, 60, 60);  // 60x60 rectangle at (100, 100)
```

### 2. Circles
```cpp
addCircle(centerX, centerY, radius);
// Example:
addCircle(400, 300, 50);  // Circle with radius 50 at center (400, 300)
```

### 3. Triangles
```cpp
addTriangle(x1, y1, x2, y2, x3, y3);
// Example:
addTriangle(100, 100, 200, 100, 150, 200);  // Triangle with 3 vertices
```

### 4. Hexagons
```cpp
addHexagon(centerX, centerY, radius);
// Example:
addHexagon(400, 300, 50);  // Hexagon with radius 50
```

### 5. Octagons
```cpp
addOctagon(centerX, centerY, radius);
// Example:
addOctagon(400, 300, 50);  // Octagon with radius 50
```

### 6. Custom Polygons
```cpp
// Create a polygon with custom vertices
std::vector<Point> customShape = {
    Point(100, 100),
    Point(200, 100),
    Point(200, 200),
    Point(150, 250),
    Point(100, 200)
};
addPolygon(customShape);
```

### 7. Polygons from Center Point
```cpp
// Create polygon with vertices relative to center
std::vector<Point> relativeVerts = {
    Point(-30, -30),  // Relative to center
    Point(30, -30),
    Point(30, 30),
    Point(-30, 30)
};
addPolygonFromCenter(400, 300, relativeVerts);  // Centered at (400, 300)
```

## Map Design Tips

### Using Relative Coordinates
Instead of hardcoding pixel values, use percentages of map size:
```cpp
float centerX = width / 2.0f;   // Center of map
float centerY = height / 2.0f;
float left = width * 0.2f;        // 20% from left
float right = width * 0.8f;       // 80% from left
float top = height * 0.8f;        // 80% from bottom
float bottom = height * 0.2f;     // 20% from bottom
```

### Creating Symmetrical Maps
```cpp
// Left side
addRectangle(width * 0.2f, height * 0.3f, 60, 60);
// Right side (mirrored)
addRectangle(width * 0.8f, height * 0.3f, 60, 60);
```

### Creating Maze-like Structures
```cpp
// Vertical walls
addRectangle(width * 0.3f, height * 0.2f, 20, 200);
addRectangle(width * 0.7f, height * 0.2f, 20, 200);

// Horizontal walls
addRectangle(width * 0.2f, height * 0.5f, 200, 20);
addRectangle(width * 0.6f, height * 0.5f, 200, 20);
```

### Creating Arena-style Maps
```cpp
// Outer ring of obstacles
for (int i = 0; i < 8; i++) {
    float angle = (i * 2.0f * 3.14159f) / 8.0f;
    float radius = 250.0f;
    addCircle(centerX + cos(angle) * radius, 
              centerY + sin(angle) * radius, 40);
}

// Center obstacle
addCircle(centerX, centerY, 80);
```

## Example Map Designs

### Simple Arena
```cpp
void Map::initializeObstacles() {
    obstacles.clear();
    
    // Borders
    float borderSize = 20.0f;
    addRectangle(0, 0, borderSize, height);
    addRectangle(width - borderSize, 0, borderSize, height);
    addRectangle(0, 0, width, borderSize);
    addRectangle(0, height - borderSize, width, borderSize);
    
    // Center circle
    addCircle(width / 2.0f, height / 2.0f, 100.0f);
    
    // Four corner pillars
    float cornerSize = 50.0f;
    addRectangle(cornerSize, cornerSize, cornerSize, cornerSize);
    addRectangle(width - cornerSize * 2, cornerSize, cornerSize, cornerSize);
    addRectangle(cornerSize, height - cornerSize * 2, cornerSize, cornerSize);
    addRectangle(width - cornerSize * 2, height - cornerSize * 2, cornerSize, cornerSize);
}
```

### Complex Maze
```cpp
void Map::initializeObstacles() {
    obstacles.clear();
    
    // Borders
    float borderSize = 20.0f;
    addRectangle(0, 0, borderSize, height);
    addRectangle(width - borderSize, 0, borderSize, height);
    addRectangle(0, 0, width, borderSize);
    addRectangle(0, height - borderSize, width, borderSize);
    
    // Create walls
    addRectangle(width * 0.3f, height * 0.1f, 15, height * 0.4f);
    addRectangle(width * 0.7f, height * 0.5f, 15, height * 0.4f);
    addRectangle(width * 0.1f, height * 0.3f, width * 0.3f, 15);
    addRectangle(width * 0.6f, height * 0.7f, width * 0.3f, 15);
    
    // Add some variety with shapes
    addHexagon(width * 0.5f, height * 0.2f, 40);
    addOctagon(width * 0.5f, height * 0.8f, 40);
}
```

## Notes

- Always keep borders (walls) to prevent players from going out of bounds
- Leave enough space between obstacles for players to move
- Test spawn positions - players won't spawn inside obstacles
- Use a mix of shapes for visual variety
- Consider gameplay flow - don't create dead ends that trap players

## Testing Your Design

1. Edit `initializeObstacles()` in `Map.cpp`
2. Rebuild the project
3. Run the game and create a room
4. Start a match to see your design
5. Adjust positions and sizes as needed

Happy map designing!

