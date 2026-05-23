/**
 * MapRenderer - Handles map display and visualization in Processing
 * 
 * Responsible for:
 * - Drawing the grid/coordinate system
 * - Rendering robot position and heading
 * - Displaying obstacles
 * - Showing scan points and sensor data
 * - Zoom and pan controls
 */

class MapRenderer {
  private float mapWidth, mapHeight;
  private float cellSize;
  private float scale;  // pixels per meter
  private float panX, panY;
  private int screenWidth, screenHeight;
  
  // Drawing parameters
  private final float GRID_COLOR = 200;
  private final float GRID_LINE_WIDTH = 0.5;
  private final float ROBOT_RADIUS = 0.1;
  private final float SCAN_POINT_SIZE = 2;
  
  // Default zoom
  private final float MIN_SCALE = 50;   // min pixels per meter
  private final float MAX_SCALE = 500;  // max pixels per meter
  private final float DEFAULT_SCALE = 150;
  
  MapRenderer(int screenW, int screenH) {
    this.screenWidth = screenW;
    this.screenHeight = screenH;
    this.scale = DEFAULT_SCALE;
    this.panX = screenW / 2.0;
    this.panY = screenH / 2.0;
    this.mapWidth = 2.0;
    this.mapHeight = 2.0;
    this.cellSize = 0.05;
  }
  
  void setMapDimensions(float width, float height, float cellSize) {
    this.mapWidth = width;
    this.mapHeight = height;
    this.cellSize = cellSize;
  }
  
  void display() {
    // Draw map background
    fill(255);
    stroke(0);
    strokeWeight(2);
    
    // Calculate map boundaries in screen coordinates
    float mapLeft = panX - (mapWidth / 2) * scale;
    float mapTop = panY - (mapHeight / 2) * scale;
    float mapRight = panX + (mapWidth / 2) * scale;
    float mapBottom = panY + (mapHeight / 2) * scale;
    
    // Draw map area
    rect(mapLeft, mapTop, mapRight - mapLeft, mapBottom - mapTop);
    
    // Draw grid
    drawGrid();
    
    // Draw origin marker
    drawOrigin();
  }
  
  void drawGrid() {
    stroke(GRID_COLOR);
    strokeWeight(GRID_LINE_WIDTH);
    
    float mapLeft = panX - (mapWidth / 2) * scale;
    float mapTop = panY - (mapHeight / 2) * scale;
    
    // Draw vertical grid lines
    for (float x = 0; x <= mapWidth; x += cellSize) {
      float screenX = mapLeft + x * scale;
      line(screenX, panY - (mapHeight / 2) * scale, 
           screenX, panY + (mapHeight / 2) * scale);
    }
    
    // Draw horizontal grid lines
    for (float y = 0; y <= mapHeight; y += cellSize) {
      float screenY = mapTop + y * scale;
      line(panX - (mapWidth / 2) * scale, screenY,
           panX + (mapWidth / 2) * scale, screenY);
    }
    
    // Draw major grid lines every 0.5m
    stroke(150);
    strokeWeight(1);
    for (float x = 0; x <= mapWidth; x += 0.5) {
      float screenX = panX - (mapWidth / 2) * scale + x * scale;
      line(screenX, panY - (mapHeight / 2) * scale,
           screenX, panY + (mapHeight / 2) * scale);
    }
    
    for (float y = 0; y <= mapHeight; y += 0.5) {
      float screenY = panY - (mapHeight / 2) * scale + y * scale;
      line(panX - (mapWidth / 2) * scale, screenY,
           panX + (mapWidth / 2) * scale, screenY);
    }
  }
  
  void drawOrigin() {
    float mapLeft = panX - (mapWidth / 2) * scale;
    float mapTop = panY - (mapHeight / 2) * scale;
    
    float screenX = mapLeft;
    float screenY = mapTop;
    
    fill(0);
    stroke(0);
    strokeWeight(2);
    ellipse(screenX, screenY, 8, 8);
    
    // Draw origin label
    fill(0);
    textAlign(LEFT);
    textSize(10);
    text("(0,0)", screenX + 5, screenY + 5);
  }
  
  void drawRobot(RobotState robot) {
    float screenX = worldToScreenX(robot.x);
    float screenY = worldToScreenY(robot.y);
    
    // Draw robot body
    fill(0, 200, 0);
    stroke(0, 100, 0);
    strokeWeight(2);
    float robotSize = ROBOT_RADIUS * 2 * scale;
    ellipse(screenX, screenY, robotSize, robotSize);
    
    // Draw heading indicator
    float angleRad = radians(robot.angle);
    float headingLen = ROBOT_RADIUS * 1.5 * scale;
    float endX = screenX + cos(angleRad) * headingLen;
    float endY = screenY + sin(angleRad) * headingLen;
    
    stroke(0, 100, 0);
    strokeWeight(3);
    line(screenX, screenY, endX, endY);
    
    // Draw velocity vector if moving
    if (robot.speed > 0.01) {
      stroke(0, 100, 200);
      strokeWeight(2);
      float velLen = robot.speed * 0.5 * scale;
      float velEndX = screenX + cos(angleRad) * velLen;
      float velEndY = screenY + sin(angleRad) * velLen;
      line(screenX, screenY, velEndX, velEndY);
    }
  }
  
  void drawObstacles(ArrayList<Obstacle> obstacles) {
    for (Obstacle obs : obstacles) {
      float screenX = worldToScreenX(obs.x);
      float screenY = worldToScreenY(obs.y);
      
      // Color based on confidence
      int alpha = (int)(obs.confidence * 2.55);  // 0-100 -> 0-255
      fill(255, 0, 0, alpha);
      stroke(200, 0, 0);
      strokeWeight(2);
      
      float rectWidth = obs.width * scale;
      float rectHeight = obs.height * scale;
      
      rectMode(CENTER);
      rect(screenX, screenY, rectWidth, rectHeight);
      
      // Draw distance text
      fill(0);
      textAlign(CENTER);
      textSize(10);
      text(String.format("%.2f m", obs.distance), screenX, screenY - rectHeight/2 - 10);
    }
  }
  
  void drawScanPoints(ArrayList<ScanPoint> scanPoints) {
    for (ScanPoint point : scanPoints) {
      if (!point.valid) continue;
      
      float screenX = worldToScreenX(point.x);
      float screenY = worldToScreenY(point.y);
      
      fill(0, 0, 255);
      stroke(0, 0, 200);
      strokeWeight(1);
      ellipse(screenX, screenY, SCAN_POINT_SIZE, SCAN_POINT_SIZE);
    }
  }
  
  // Coordinate conversion helpers
  float worldToScreenX(float worldX) {
    float mapLeft = panX - (mapWidth / 2) * scale;
    return mapLeft + worldX * scale;
  }
  
  float worldToScreenY(float worldY) {
    float mapTop = panY - (mapHeight / 2) * scale;
    return mapTop + worldY * scale;
  }
  
  float screenToWorldX(float screenX) {
    float mapLeft = panX - (mapWidth / 2) * scale;
    return (screenX - mapLeft) / scale;
  }
  
  float screenToWorldY(float screenY) {
    float mapTop = panY - (mapHeight / 2) * scale;
    return (screenY - mapTop) / scale;
  }
  
  void zoomIn() {
    scale = min(scale * 1.2, MAX_SCALE);
  }
  
  void zoomOut() {
    scale = max(scale / 1.2, MIN_SCALE);
  }
  
  void resetView() {
    scale = DEFAULT_SCALE;
    panX = screenWidth / 2.0;
    panY = screenHeight / 2.0;
  }
  
  void pan(float deltaX, float deltaY) {
    panX += deltaX;
    panY += deltaY;
  }
  
  float getScale() {
    return scale;
  }
}
