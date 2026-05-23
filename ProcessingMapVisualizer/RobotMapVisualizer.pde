/**
 * Robot Map Visualizer - Processing Application
 * 
 * This Processing sketch connects to the SWARM robot via WebSocket
 * and displays real-time environmental maps with robot position,
 * obstacles, and sensor scan data.
 * 
 * Requirements:
 * - websockets library: https://github.com/getify/Processing-WebSocket
 * - Processing 4.0+
 * 
 * Installation:
 * 1. Download websockets library from Sketch > Import Library > Add Library
 * 2. Search for "websockets" and install
 * 3. Run this sketch
 */

import websockets.*;

// Configuration
final String ROBOT_IP = "192.168.4.1";
final int ROBOT_PORT = 81;

// Global variables
WebSocketClient wsc;
MapRenderer mapRenderer;
RobotState robotState;
ArrayList<Obstacle> obstacles;
ArrayList<ScanPoint> scanPoints;
JSONObject lastMapData;

boolean connected = false;
int updateCount = 0;
int fps_display = 0;
unsigned long lastUpdate = 0;

void settings() {
  size(1200, 900);
}

void setup() {
  background(255);
  frameRate(30);
  
  // Initialize data structures
  robotState = new RobotState();
  obstacles = new ArrayList<Obstacle>();
  scanPoints = new ArrayList<ScanPoint>();
  
  // Initialize map renderer
  mapRenderer = new MapRenderer(width, height);
  mapRenderer.setMapDimensions(2.0, 2.0, 0.05);
  
  // Connect to robot WebSocket
  connectToRobot();
}

void draw() {
  background(240);
  
  // Draw the map
  mapRenderer.display();
  mapRenderer.drawRobot(robotState);
  mapRenderer.drawScanPoints(scanPoints);
  mapRenderer.drawObstacles(obstacles);
  
  // Draw UI
  drawUI();
  
  fps_display = frameRate > 0 ? (int)frameRate : 0;
}

void drawUI() {
  fill(0);
  textSize(14);
  textAlign(LEFT);
  
  // Connection status
  String status = connected ? "CONNECTED" : "DISCONNECTED";
  fill(connected ? color(0, 200, 0) : color(200, 0, 0));
  rect(10, 10, 200, 25);
  fill(255);
  text("Status: " + status, 20, 30);
  
  // Robot position and angle
  fill(0);
  text("Robot X: " + String.format("%.2f", robotState.x) + " m", 10, 60);
  text("Robot Y: " + String.format("%.2f", robotState.y) + " m", 10, 80);
  text("Angle: " + String.format("%.1f", robotState.angle) + "°", 10, 100);
  text("Speed: " + String.format("%.2f", robotState.speed) + " m/s", 10, 120);
  
  // Mode indicator
  String mode = getModeString(robotState.mode);
  text("Mode: " + mode, 10, 140);
  
  // Data statistics
  text("Obstacles: " + obstacles.size(), 10, 170);
  text("Scan Points: " + scanPoints.size(), 10, 190);
  text("Updates: " + updateCount, 10, 210);
  text("FPS: " + fps_display, 10, 230);
  
  // Instructions
  fill(100);
  textSize(12);
  text("[SPACE] Clear Map | [R] Reset View | [+/-] Zoom", 10, height - 30);
  text("Press 'C' to connect | 'D' to disconnect", 10, height - 10);
}

String getModeString(int mode) {
  switch(mode) {
    case 0: return "Idle";
    case 1: return "Line Following";
    case 2: return "Obstacle Avoidance";
    default: return "Unknown";
  }
}

void connectToRobot() {
  try {
    String url = "ws://" + ROBOT_IP + ":" + ROBOT_PORT;
    println("Connecting to: " + url);
    wsc = new WebSocketClient(this, url);
    println("WebSocket client created");
  } catch (Exception e) {
    println("Error connecting: " + e.getMessage());
    connected = false;
  }
}

// WebSocket event handlers
void webSocketEvent(String msg) {
  try {
    JSONObject data = parseJson(msg);
    if (data != null) {
      String type = data.getString("type", "");
      
      if (type.equals("map_update")) {
        handleMapUpdate(data);
      }
    }
  } catch (Exception e) {
    println("Error parsing message: " + e.getMessage());
  }
}

void handleMapUpdate(JSONObject data) {
  updateCount++;
  lastUpdate = millis();
  
  try {
    // Update robot state
    if (data.hasKey("robot")) {
      JSONObject robot = data.getJSONObject("robot");
      robotState.x = (float)robot.getDouble("x", 0);
      robotState.y = (float)robot.getDouble("y", 0);
      robotState.angle = (float)robot.getDouble("angle", 0);
      robotState.speed = (float)robot.getDouble("speed", 0);
      robotState.mode = robot.getInt("mode", 0);
    }
    
    // Clear and update obstacles
    obstacles.clear();
    if (data.hasKey("obstacles")) {
      JSONArray obsArray = data.getJSONArray("obstacles");
      for (int i = 0; i < obsArray.size(); i++) {
        JSONObject obj = obsArray.getJSONObject(i);
        Obstacle obs = new Obstacle(
          (float)obj.getDouble("x", 0),
          (float)obj.getDouble("y", 0),
          (float)obj.getDouble("distance", 0),
          (float)obj.getDouble("width", 0.1),
          (float)obj.getDouble("height", 0.1),
          obj.getInt("confidence", 0)
        );
        obstacles.add(obs);
      }
    }
    
    // Clear and update scan points
    scanPoints.clear();
    if (data.hasKey("scanPoints")) {
      JSONArray scanArray = data.getJSONArray("scanPoints");
      for (int i = 0; i < scanArray.size(); i++) {
        JSONObject scan = scanArray.getJSONObject(i);
        ScanPoint point = new ScanPoint(
          (float)scan.getDouble("x", 0),
          (float)scan.getDouble("y", 0),
          (float)scan.getDouble("distance", 0),
          (float)scan.getDouble("angle", 0),
          scan.getBoolean("valid", false)
        );
        scanPoints.add(point);
      }
    }
    
    // Update map dimensions if provided
    if (data.hasKey("mapWidth") && data.hasKey("mapHeight")) {
      float w = (float)data.getDouble("mapWidth", 2.0);
      float h = (float)data.getDouble("mapHeight", 2.0);
      float cell = (float)data.getDouble("cellSize", 0.05);
      mapRenderer.setMapDimensions(w, h, cell);
    }
    
  } catch (Exception e) {
    println("Error updating map: " + e.getMessage());
  }
}

void keyPressed() {
  if (key == 'c' || key == 'C') {
    connectToRobot();
  } else if (key == 'd' || key == 'D') {
    if (wsc != null) {
      wsc.close();
    }
    connected = false;
  } else if (key == ' ') {
    // Clear map
    obstacles.clear();
    scanPoints.clear();
    println("Map cleared");
  } else if (key == 'r' || key == 'R') {
    // Reset view
    mapRenderer.resetView();
    println("View reset");
  } else if (key == '+' || key == '=') {
    mapRenderer.zoomIn();
  } else if (key == '-' || key == '_') {
    mapRenderer.zoomOut();
  }
}

// JSON parsing helper
JSONObject parseJson(String jsonString) {
  try {
    return JSONObject.parse(jsonString);
  } catch (Exception e) {
    return null;
  }
}

// Data structure classes

class RobotState {
  float x, y;
  float angle;
  float speed;
  int mode;
  
  RobotState() {
    this.x = 0;
    this.y = 0;
    this.angle = 0;
    this.speed = 0;
    this.mode = 0;
  }
}

class Obstacle {
  float x, y;
  float distance;
  float width, height;
  int confidence;
  
  Obstacle(float x, float y, float dist, float w, float h, int conf) {
    this.x = x;
    this.y = y;
    this.distance = dist;
    this.width = w;
    this.height = h;
    this.confidence = conf;
  }
}

class ScanPoint {
  float x, y;
  float distance;
  float angle;
  boolean valid;
  
  ScanPoint(float x, float y, float dist, float angle, boolean valid) {
    this.x = x;
    this.y = y;
    this.distance = dist;
    this.angle = angle;
    this.valid = valid;
  }
}
