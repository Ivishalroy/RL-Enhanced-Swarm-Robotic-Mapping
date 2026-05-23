#ifndef MAPBUILDER_H
#define MAPBUILDER_H

#include <ArduinoJson.h>
#include <math.h>

// Map grid cell
struct MapCell {
  int obstacleCount;    // Number of obstacle detections
  int freeCount;        // Number of free space detections
  uint16_t lastDistance;// Last measured distance in mm
  unsigned long lastUpdate;
};

class MapBuilder {
private:
  static const int GRID_SIZE = 41;          // 41x41 grid
  static const int CELL_SIZE_MM = 50;       // Each cell is 50mm x 50mm
  static const int MAP_WIDTH_MM = GRID_SIZE * CELL_SIZE_MM;  // 2050mm total
  
  MapCell grid[GRID_SIZE][GRID_SIZE];
  
  // Robot state
  float robotX;        // Robot X position in mm (center of map)
  float robotY;        // Robot Y position in mm (center of map)
  float robotAngle;    // Robot heading in degrees (0 = facing +X direction)
  
  // Map bounds for normalization
  float mapCenterX;
  float mapCenterY;
  
  unsigned long mapCreatedTime;
  
public:
  MapBuilder() 
    : robotX(MAP_WIDTH_MM / 2.0), 
      robotY(MAP_WIDTH_MM / 2.0), 
      robotAngle(0.0),
      mapCenterX(MAP_WIDTH_MM / 2.0),
      mapCenterY(MAP_WIDTH_MM / 2.0),
      mapCreatedTime(millis()) {
    initializeGrid();
  }

  void initializeGrid() {
    for (int i = 0; i < GRID_SIZE; i++) {
      for (int j = 0; j < GRID_SIZE; j++) {
        grid[i][j].obstacleCount = 0;
        grid[i][j].freeCount = 0;
        grid[i][j].lastDistance = 0;
        grid[i][j].lastUpdate = 0;
      }
    }
  }

  void updateRobotPose(float x, float y, float angle) {
    robotX = x;
    robotY = y;
    robotAngle = angle;
  }

  void getRobotPose(float &x, float &y, float &angle) {
    x = robotX;
    y = robotY;
    angle = robotAngle;
  }

  void addMeasurement(uint16_t distanceMM, float sensorOffsetX, float sensorOffsetY, float sensorAngle) {
    // Calculate absolute sensor position
    float angleRad = robotAngle * M_PI / 180.0;
    float sensorAbsX = robotX + sensorOffsetX * cos(angleRad) - sensorOffsetY * sin(angleRad);
    float sensorAbsY = robotY + sensorOffsetX * sin(angleRad) + sensorOffsetY * cos(angleRad);
    
    // Calculate absolute sensor angle
    float sensorAbsAngle = robotAngle + sensorAngle;
    float sensorAbsAngleRad = sensorAbsAngle * M_PI / 180.0;
    
    // Mark cells along the ray
    int maxRange = min((uint16_t)1500, distanceMM);  // Limit range for processing
    int raySteps = maxRange / CELL_SIZE_MM;
    
    // Mark cells as free space up to the obstacle
    for (int step = 1; step < raySteps; step++) {
      float x = sensorAbsX + step * CELL_SIZE_MM * cos(sensorAbsAngleRad);
      float y = sensorAbsY + step * CELL_SIZE_MM * sin(sensorAbsAngleRad);
      
      int gridX = (int)((x - mapCenterX + MAP_WIDTH_MM/2.0) / CELL_SIZE_MM);
      int gridY = (int)((y - mapCenterY + MAP_WIDTH_MM/2.0) / CELL_SIZE_MM);
      
      if (isValidGridPos(gridX, gridY)) {
        grid[gridX][gridY].freeCount++;
        grid[gridX][gridY].lastUpdate = millis();
      }
    }
    
    // Mark obstacle cell
    float obsX = sensorAbsX + (distanceMM - 20) * cos(sensorAbsAngleRad);  // -20mm offset for object center
    float obsY = sensorAbsY + (distanceMM - 20) * sin(sensorAbsAngleRad);
    
    int obsGridX = (int)((obsX - mapCenterX + MAP_WIDTH_MM/2.0) / CELL_SIZE_MM);
    int obsGridY = (int)((obsY - mapCenterY + MAP_WIDTH_MM/2.0) / CELL_SIZE_MM);
    
    if (isValidGridPos(obsGridX, obsGridY)) {
      grid[obsGridX][obsGridY].obstacleCount++;
      grid[obsGridX][obsGridY].lastDistance = distanceMM;
      grid[obsGridX][obsGridY].lastUpdate = millis();
    }
  }

  void addRobotPosition(int gridX, int gridY) {
    // Mark robot position (with uncertainty radius)
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        if (isValidGridPos(gridX + i, gridY + j)) {
          grid[gridX + i][gridY + j].freeCount++;
        }
      }
    }
  }

  bool isValidGridPos(int x, int y) {
    return (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE);
  }

  void getRobotGridPos(int &gridX, int &gridY) {
    gridX = (int)((robotX - mapCenterX + MAP_WIDTH_MM/2.0) / CELL_SIZE_MM);
    gridY = (int)((robotY - mapCenterY + MAP_WIDTH_MM/2.0) / CELL_SIZE_MM);
  }

  // Get map as JSON for web interface
  String getMapJSON() {
    DynamicJsonDocument doc(16384);
    
    // Get robot position
    int robotGridX, robotGridY;
    getRobotGridPos(robotGridX, robotGridY);
    
    JsonArray mapArray = doc.createNestedArray("map");
    
    for (int i = 0; i < GRID_SIZE; i++) {
      JsonArray row = mapArray.createNestedArray();
      for (int j = 0; j < GRID_SIZE; j++) {
        int cellType = 0;  // 0 = unknown, 1 = free, 2 = obstacle
        
        if (grid[i][j].obstacleCount > 0) {
          cellType = 2;
        } else if (grid[i][j].freeCount > 0) {
          cellType = 1;
        }
        
        row.add(cellType);
      }
    }
    
    doc["robotX"] = robotGridX;
    doc["robotY"] = robotGridY;
    doc["robotAngle"] = robotAngle;
    doc["gridSize"] = GRID_SIZE;
    doc["cellSizeMM"] = CELL_SIZE_MM;
    doc["timestamp"] = millis() - mapCreatedTime;
    
    String json;
    serializeJson(doc, json);
    return json;
  }

  // Get simplified map for debugging
  void printMapStats() {
    int obstacles = 0;
    int free = 0;
    
    for (int i = 0; i < GRID_SIZE; i++) {
      for (int j = 0; j < GRID_SIZE; j++) {
        if (grid[i][j].obstacleCount > 0) obstacles++;
        if (grid[i][j].freeCount > 0) free++;
      }
    }
    
    Serial.print("Map - Obstacles: ");
    Serial.print(obstacles);
    Serial.print(", Free: ");
    Serial.println(free);
  }

  void resetMap() {
    initializeGrid();
    mapCreatedTime = millis();
  }

  float getMapCoveragePercent() {
    int mapped = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
      for (int j = 0; j < GRID_SIZE; j++) {
        if (grid[i][j].obstacleCount > 0 || grid[i][j].freeCount > 0) {
          mapped++;
        }
      }
    }
    return (mapped * 100.0) / (GRID_SIZE * GRID_SIZE);
  }
};

#endif
