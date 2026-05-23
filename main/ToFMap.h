#ifndef TOF_MAP_H
#define TOF_MAP_H

#include "ToFSensor.h"

class ToFMap {
private:
  // Map configuration
  static const int MAP_SIZE = 100;           // 100x100 grid
  static const int RESOLUTION = 50;          // 50mm per cell
  static const int MAX_RANGE = 2000;         // Max distance in mm (2 meters)
  
  // Map data: 0=unknown, 1=obstacle, 2=free space
  uint8_t gridMap[MAP_SIZE][MAP_SIZE];
  
  // Current sensor position and orientation
  float robotX = MAP_SIZE / 2;               // Robot X position in grid (center)
  float robotY = MAP_SIZE / 2;               // Robot Y position in grid (center)
  float robotAngle = 0;                      // Robot heading in degrees
  
  // Scan data for transmission
  struct ScanData {
    int angle;
    int distance;
  } scanBuffer[360];
  int scanCount = 0;
  
public:
  ToFMap() {
    // Initialize map as unknown (all cells = 0)
    for (int i = 0; i < MAP_SIZE; i++) {
      for (int j = 0; j < MAP_SIZE; j++) {
        gridMap[i][j] = 0;
      }
    }
  }

  // Add a distance reading at current angle
  void addMeasurement(float angle, uint16_t distance) {
    if (distance == 0 || distance > MAX_RANGE) {
      return;  // Invalid reading
    }

    // Convert to radians
    float radians = angle * 3.14159265 / 180.0;
    
    // Calculate endpoint in sensor frame
    float sensorX = (distance / RESOLUTION) * cos(radians);
    float sensorY = (distance / RESOLUTION) * sin(radians);
    
    // Transform to world frame (robot-centric)
    float robotRadians = robotAngle * 3.14159265 / 180.0;
    float worldX = robotX + sensorX * cos(robotRadians) - sensorY * sin(robotRadians);
    float worldY = robotY + sensorX * sin(robotRadians) + sensorY * cos(robotRadians);
    
    // Clamp to map boundaries
    int gridX = constrain((int)worldX, 0, MAP_SIZE - 1);
    int gridY = constrain((int)worldY, 0, MAP_SIZE - 1);
    
    // Mark cell as obstacle
    if (gridX >= 0 && gridX < MAP_SIZE && gridY >= 0 && gridY < MAP_SIZE) {
      gridMap[gridX][gridY] = 1;  // Obstacle
    }
    
    // Bresenham line tracing for free space (optionally mark cells between robot and obstacle)
    traceFreePath(robotX, robotY, worldX, worldY);
  }

  // Trace free space between robot and obstacle
  void traceFreePath(float x0, float y0, float x1, float y1) {
    int dx = abs((int)x1 - (int)x0);
    int dy = abs((int)y1 - (int)y0);
    int sx = ((int)x1 > (int)x0) ? 1 : -1;
    int sy = ((int)y1 > (int)y0) ? 1 : -1;
    int err = dx - dy;

    int x = (int)x0;
    int y = (int)y0;

    while (true) {
      // Mark as free space (only if not already marked as obstacle)
      if (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE) {
        if (gridMap[x][y] == 0) {
          gridMap[x][y] = 2;  // Free space
        }
      }

      if (x == (int)x1 && y == (int)y1) break;

      int e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x += sx;
      }
      if (e2 < dx) {
        err += dx;
        y += sy;
      }
    }
  }

  // Update robot position
  void setRobotPosition(float x, float y) {
    robotX = MAP_SIZE / 2 + (x / RESOLUTION);
    robotY = MAP_SIZE / 2 + (y / RESOLUTION);
  }

  // Update robot angle
  void setRobotAngle(float angle) {
    robotAngle = angle;
  }

  // Get map data as JSON string for web transmission
  String getMapJSON() {
    String json = "{\"map\":[";
    
    for (int i = 0; i < MAP_SIZE; i++) {
      for (int j = 0; j < MAP_SIZE; j++) {
        json += (int)gridMap[i][j];
        if (!(i == MAP_SIZE - 1 && j == MAP_SIZE - 1)) {
          json += ",";
        }
      }
    }
    
    json += "],\"robot\":{\"x\":" + String((int)robotX) + 
            ",\"y\":" + String((int)robotY) + 
            ",\"angle\":" + String((int)robotAngle) + 
            "},\"size\":" + String(MAP_SIZE) + "}";
    
    return json;
  }

  // Get compact map data (only non-zero cells) for efficiency
  String getCompactMapJSON() {
    String json = "{\"obstacles\":[";
    int obstacleCount = 0;
    
    // Count obstacles first
    for (int i = 0; i < MAP_SIZE; i++) {
      for (int j = 0; j < MAP_SIZE; j++) {
        if (gridMap[i][j] == 1) obstacleCount++;
      }
    }
    
    // Only include data if we have measurements
    if (obstacleCount > 0) {
      bool first = true;
      for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
          if (gridMap[i][j] == 1) {
            if (!first) json += ",";
            json += "{\"x\":" + String(i) + ",\"y\":" + String(j) + "}";
            first = false;
          }
        }
      }
    }
    
    json += "],\"free\":[";
    int freeCount = 0;
    
    // Only send some free cells to reduce size
    for (int i = 0; i < MAP_SIZE; i++) {
      for (int j = 0; j < MAP_SIZE; j++) {
        if (gridMap[i][j] == 2) freeCount++;
      }
    }
    
    if (freeCount > 0) {
      bool first = true;
      for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
          if (gridMap[i][j] == 2) {
            if (!first) json += ",";
            json += "{\"x\":" + String(i) + ",\"y\":" + String(j) + "}";
            first = false;
          }
        }
      }
    }
    
    json += "],\"robot\":{\"x\":" + String((int)robotX) + 
            ",\"y\":" + String((int)robotY) + 
            ",\"angle\":" + String((int)robotAngle) + 
            "},\"size\":" + String(MAP_SIZE) + 
            ",\"obstacles_count\":" + String(obstacleCount) +
            ",\"free_count\":" + String(freeCount) + "}";
    
    return json;
  }

  // Clear map
  void clearMap() {
    for (int i = 0; i < MAP_SIZE; i++) {
      for (int j = 0; j < MAP_SIZE; j++) {
        gridMap[i][j] = 0;
      }
    }
  }

  // Get map size
  int getMapSize() {
    return MAP_SIZE;
  }

  // Get cell value
  uint8_t getCell(int x, int y) {
    if (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE) {
      return gridMap[x][y];
    }
    return 0;
  }
};

#endif
