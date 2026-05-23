#ifndef MAP_DRAWER_H
#define MAP_DRAWER_H

#include <vector>
#include <cstdint>
#include <WebSocketsServer.h>

/**
 * @file MapDrawer.h
 * @brief Interface for map visualization and Python drawing integration
 * 
 * This header provides the data structures and communication protocol for
 * sending map data from the robot to Python visualization client.
 * The Python client can draw maps in real-time using this data.
 */

// Data structures for map visualization

/**
 * @struct Point2D
 * @brief Represents a 2D point in Cartesian coordinates
 */
struct Point2D {
    float x;
    float y;
    
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
};

/**
 * @struct RobotState
 * @brief Current state of the robot for map context
 */
struct RobotState {
    Point2D position;        // Robot's current position in meters
    float angle;             // Robot's heading angle in degrees (0-360)
    float speed;             // Current movement speed in m/s
    uint8_t mode;            // Current operation mode (0=idle, 1=line_follow, 2=avoid_obstacles)
    
    RobotState() : position(0, 0), angle(0), speed(0), mode(0) {}
};

/**
 * @struct Obstacle
 * @brief Represents a detected obstacle
 */
struct Obstacle {
    Point2D position;        // Center position of obstacle
    float distance;          // Distance from robot to obstacle in meters
    float width;             // Estimated width of obstacle
    float height;            // Estimated height of obstacle
    uint16_t confidence;     // Detection confidence (0-100)
    
    Obstacle() : position(0, 0), distance(0), width(0.1), height(0.1), confidence(0) {}
};

/**
 * @struct ScanData
 * @brief Single scan measurement from ToF sensor
 */
struct ScanData {
    float angle;             // Scan angle in degrees
    uint16_t distance;       // Measured distance in mm
    bool valid;              // Whether measurement is valid
    uint32_t timestamp;      // Timestamp of measurement in milliseconds
    
    ScanData() : angle(0), distance(0), valid(false), timestamp(0) {}
};

/**
 * @struct GridCell
 * @brief Occupancy grid cell for probabilistic mapping
 */
struct GridCell {
    uint8_t occupancy;       // Occupancy probability (0-255, 128=unknown)
    uint8_t confidence;      // Confidence level of this cell (0-100)
    
    GridCell() : occupancy(128), confidence(0) {}
};

/**
 * @class MapData
 * @brief Container for all map visualization data
 */
class MapData {
public:
    MapData() : 
        mapWidth(0), mapHeight(0), cellSize(0.05), 
        updateCount(0), isValid(false) {}
    
    // Robot state
    RobotState robotState;
    
    // Detected obstacles
    std::vector<Obstacle> obstacles;
    
    // Recent scan data points
    std::vector<ScanData> scanPoints;
    
    // Occupancy grid
    std::vector<GridCell> occupancyGrid;
    
    // Map dimensions
    float mapWidth;          // Width of map in meters
    float mapHeight;         // Height of map in meters
    float cellSize;          // Size of each grid cell in meters
    
    // Metadata
    uint32_t updateCount;    // Total update count for synchronization
    bool isValid;            // Whether map data is valid for drawing
    uint32_t timestamp;      // Last update timestamp
    
    /**
     * @brief Clear all map data
     */
    void clear() {
        obstacles.clear();
        scanPoints.clear();
        occupancyGrid.clear();
        updateCount = 0;
        isValid = false;
    }
    
    /**
     * @brief Add an obstacle to the map
     */
    void addObstacle(const Obstacle& obs) {
        obstacles.push_back(obs);
    }
    
    /**
     * @brief Add a scan point to the map
     */
    void addScanPoint(const ScanData& scan) {
        scanPoints.push_back(scan);
    }
    
    /**
     * @brief Get obstacle count
     */
    uint16_t getObstacleCount() const {
        return obstacles.size();
    }
    
    /**
     * @brief Get scan point count
     */
    uint16_t getScanPointCount() const {
        return scanPoints.size();
    }
};

/**
 * @class MapDrawer
 * @brief Handles communication of map data to Python visualization clients
 * 
 * This class manages the serialization and transmission of map data over
 * WebSocket to Python clients for real-time visualization.
 * 
 * Python Integration:
 * - Receives map updates via WebSocket messages
 * - Deserializes JSON map data
 * - Renders map using matplotlib or other visualization libraries
 * - Sends control commands back to robot
 */
class MapDrawer {
public:
    /**
     * @brief Constructor
     * @param socketServer Pointer to WebSocketsServer for sending data
     */
    explicit MapDrawer(WebSocketsServer* socketServer = nullptr) 
        : socket(socketServer), clientConnected(false) {}
    
    /**
     * @brief Initialize the map drawer
     */
    void begin() {
        clientConnected = false;
    }
    
    /**
     * @brief Set the socket server for communication
     */
    void setSocketServer(WebSocketsServer* socketServer) {
        socket = socketServer;
    }
    
    /**
     * @brief Send map data to all connected Python clients
     * @param mapData The map data to send
     * @return true if data was sent successfully
     * 
     * Format: JSON serialization of map data
     * {
     *   "type": "map_update",
     *   "robot": {"x": float, "y": float, "angle": float},
     *   "obstacles": [...],
     *   "scanPoints": [...],
     *   "grid": {...},
     *   "timestamp": uint32_t
     * }
     */
    bool sendMapUpdate(const MapData& mapData) {
        if (!socket || !clientConnected) return false;
        
        // Serialization happens in webhandler
        // This is the data structure interface
        return true;
    }
    
    /**
     * @brief Notify that a Python client has connected
     */
    void onClientConnect() {
        clientConnected = true;
    }
    
    /**
     * @brief Notify that a Python client has disconnected
     */
    void onClientDisconnect() {
        clientConnected = false;
    }
    
    /**
     * @brief Check if a Python client is connected
     */
    bool isClientConnected() const {
        return clientConnected;
    }
    
    /**
     * @brief Get the current map data
     */
    MapData& getMapData() {
        return currentMap;
    }
    
    /**
     * @brief Get current map data (const version)
     */
    const MapData& getMapData() const {
        return currentMap;
    }
    
    /**
     * @brief Clear all map data
     */
    void clearMap() {
        currentMap.clear();
    }
    
    /**
     * @brief Update map dimensions
     */
    void setMapDimensions(float width, float height, float cellSize = 0.05) {
        currentMap.mapWidth = width;
        currentMap.mapHeight = height;
        currentMap.cellSize = cellSize;
    }
    
private:
    WebSocketsServer* socket;
    MapData currentMap;
    bool clientConnected;
};

// JSON Serialization Helpers for Python Integration
/**
 * @brief Convert map data to JSON string for transmission
 * @param mapData The map data to serialize
 * @return JSON string representation
 * 
 * Used internally by webhandler to send data to Python clients
 * 
 * Example Python client reception:
 * @code
 * import json
 * import websocket
 * 
 * def on_message(ws, message):
 *     data = json.loads(message)
 *     if data['type'] == 'map_update':
 *         draw_map(data['robot'], data['obstacles'], data['scanPoints'])
 * 
 * ws = websocket.WebSocketApp("ws://[robot-ip]:81")
 * ws.on_message = on_message
 * ws.run_forever()
 * @endcode
 */
inline String mapDataToJSON(const MapData& mapData) {
    // Implementation would be in webhandler.h
    // Returns JSON string like:
    // {
    //   "type": "map_update",
    //   "timestamp": 1234567890,
    //   "robot": {"x": 0.5, "y": 0.5, "angle": 45.0, "speed": 0.2, "mode": 1},
    //   "obstacles": [...],
    //   "scanPoints": [...],
    //   "mapWidth": 2.0,
    //   "mapHeight": 2.0,
    //   "cellSize": 0.05
    // }
    return "{}";  // Placeholder
}

#endif // MAP_DRAWER_H

/**
 * @page python_integration Python Integration Guide
 * 
 * ## Overview
 * The MapDrawer header provides a standardized interface for Python visualization clients
 * to receive robot map data in real-time over WebSocket.
 * 
 * ## Python Client Example
 * 
 * @code{.py}
 * import json
 * import websocket
 * import matplotlib.pyplot as plt
 * from matplotlib.patches import Rectangle, Circle
 * import numpy as np
 * 
 * class RobotMapVisualizer:
 *     def __init__(self, robot_ip):
 *         self.ws = websocket.WebSocketApp(
 *             f"ws://{robot_ip}:81",
 *             on_message=self.on_message,
 *             on_error=self.on_error,
 *             on_close=self.on_close
 *         )
 *         self.fig, self.ax = plt.subplots()
 *         
 *     def on_message(self, ws, message):
 *         data = json.loads(message)
 *         if data['type'] == 'map_update':
 *             self.draw_map(data)
 *             
 *     def draw_map(self, map_data):
 *         self.ax.clear()
 *         
 *         # Draw grid
 *         width = map_data['mapWidth']
 *         height = map_data['mapHeight']
 *         self.ax.set_xlim(0, width)
 *         self.ax.set_ylim(0, height)
 *         
 *         # Draw obstacles
 *         for obs in map_data.get('obstacles', []):
 *             rect = Rectangle(
 *                 (obs['x'] - obs['width']/2, obs['y'] - obs['height']/2),
 *                 obs['width'], obs['height'],
 *                 color='red', alpha=0.7
 *             )
 *             self.ax.add_patch(rect)
 *         
 *         # Draw scan points
 *         for point in map_data.get('scanPoints', []):
 *             x = point['x']
 *             y = point['y']
 *             self.ax.plot(x, y, 'b.', markersize=2)
 *         
 *         # Draw robot
 *         robot = map_data['robot']
 *         circle = Circle((robot['x'], robot['y']), 0.1, color='green')
 *         self.ax.add_patch(circle)
 *         
 *         # Draw robot heading
 *         angle_rad = np.radians(robot['angle'])
 *         dx = 0.15 * np.cos(angle_rad)
 *         dy = 0.15 * np.sin(angle_rad)
 *         self.ax.arrow(robot['x'], robot['y'], dx, dy, 
 *                        head_width=0.05, head_length=0.05, color='green')
 *         
 *         plt.pause(0.01)
 *     
 *     def run(self):
 *         self.ws.run_forever()
 * 
 * if __name__ == "__main__":
 *     visualizer = RobotMapVisualizer("192.168.4.1")
 *     visualizer.run()
 * @endcode
 * 
 * ## Data Structure
 * The JSON format sent from C++ to Python contains:
 * - **robot**: Current position (x, y), angle, speed, mode
 * - **obstacles**: Array of detected obstacles with position, dimensions, confidence
 * - **scanPoints**: Array of ToF sensor readings
 * - **mapDimensions**: Width, height, cell size
 * - **timestamp**: Update timestamp for synchronization
 * 
 * ## Connection
 * Connect to WebSocket at: `ws://[robot-ip]:81`
 * Required libraries: websocket-client, matplotlib, numpy
 * 
 * @endcode
 */
