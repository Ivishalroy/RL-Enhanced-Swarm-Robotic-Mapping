# Processing Map Visualizer

A real-time visualization application for the SWARM robot's environmental mapping system. This Processing application connects to the robot via WebSocket and displays the map with robot position, obstacles, and sensor data.

## Overview

The Processing Map Visualizer receives map data from the SWARM robot in real-time and renders it in an interactive 2D display. It shows:
- Robot position and orientation
- Detected obstacles with confidence levels
- ToF sensor scan points
- Distance measurements
- Operating mode indicator
- Real-time statistics

## Features

- **Real-time Map Display**: Live updates from robot sensors
- **Interactive Visualization**: Zoom, pan, and reset controls
- **Robot Tracking**: Shows position, heading, and velocity
- **Obstacle Detection**: Visual representation of detected obstacles with confidence
- **Sensor Data**: Display of raw ToF sensor measurements
- **Grid Reference**: Configurable grid overlay for distance reference
- **Connection Status**: Visual indicator of robot connection status
- **Performance Metrics**: FPS and update count display

## Requirements

- **Processing 4.0** or higher
- **WebSockets Library** for Processing
  - Install via: Sketch > Import Library > Manage Libraries
  - Search for "websockets" by Jack Elston

## Installation

1. **Install Processing**
   - Download from https://processing.org/download

2. **Install WebSockets Library**
   - Open Processing IDE
   - Go to Sketch > Import Library > Add Library
   - Search for "WebSocket"
   - Install the "websockets" library by Jack Elston

3. **Download Visualizer Files**
   - Place all .pde files in a Processing sketch folder:
     - `RobotMapVisualizer.pde` (main sketch)
     - `MapRenderer.pde` (rendering engine)

4. **Configure Connection**
   - Edit `RobotMapVisualizer.pde`
   - Set `ROBOT_IP` to your robot's IP address (default: 192.168.4.1)
   - Set `ROBOT_PORT` (default: 81)

## Usage

### Starting the Application

1. Open `RobotMapVisualizer.pde` in Processing
2. Click the "Run" button or press Ctrl+R (Cmd+R on Mac)
3. The window will open and attempt to connect to the robot

### Controls

| Key | Action |
|-----|--------|
| `C` | Connect to robot |
| `D` | Disconnect from robot |
| `SPACE` | Clear map data |
| `R` | Reset map view |
| `+` | Zoom in |
| `-` | Zoom out |

### Display Elements

**Status Bar (Top-Left)**
- Connection status (green=connected, red=disconnected)
- Robot position (X, Y in meters)
- Robot heading angle (0-360°)
- Current speed (m/s)
- Operation mode (Idle, Line Following, Obstacle Avoidance)

**Map Area**
- Grid overlay (minor cells: 5cm, major cells: 50cm)
- Robot position (green circle with heading arrow)
- Obstacles (red rectangles with confidence shading)
- Scan points (blue dots)
- Origin marker at (0,0)

**Statistics (Bottom-Left)**
- Obstacle count
- Scan point count
- Total updates received
- Frames per second (FPS)

### Default Configuration

```
Robot IP: 192.168.4.1
Robot Port: 81
Window Size: 1200 x 900 pixels
Refresh Rate: 30 FPS
Default Zoom: 150 pixels/meter
Map Size: 2.0m x 2.0m
```

## Network Setup

### Connecting to Robot

1. Ensure robot is powered on and WiFi is active
2. Connect your computer to the robot's WiFi network:
   - SSID: `StochBot`
   - Password: `12345678`

3. The visualizer will automatically attempt connection
   - Or press `C` to manually connect
   - Status will show "CONNECTED" when link is established

### Network Requirements

- Both computer and robot must be on the same WiFi network
- WebSocket port 81 must be accessible
- Stable connection recommended for smooth visualization

## Data Format

The visualizer expects JSON-formatted map data from the robot:

```json
{
  "type": "map_update",
  "robot": {
    "x": 0.5,
    "y": 0.5,
    "angle": 45.0,
    "speed": 0.2,
    "mode": 1
  },
  "obstacles": [
    {
      "x": 1.0,
      "y": 0.8,
      "distance": 0.5,
      "width": 0.2,
      "height": 0.2,
      "confidence": 85
    }
  ],
  "scanPoints": [
    {
      "x": 0.9,
      "y": 0.7,
      "distance": 0.45,
      "angle": 45.0,
      "valid": true
    }
  ],
  "mapWidth": 2.0,
  "mapHeight": 2.0,
  "cellSize": 0.05,
  "timestamp": 1234567890
}
```

## Troubleshooting

### Connection Issues

**Can't connect to robot:**
- Verify robot is powered on
- Confirm WiFi network is active (SSID: StochBot)
- Check ROBOT_IP matches actual robot IP (use `Serial.println(WiFi.softAPIP())`)
- Ensure firewall isn't blocking port 81
- Try pressing 'C' to manually reconnect

**Connection shows "DISCONNECTED":**
- Check WiFi connection to StochBot network
- Verify robot's WebSocket server is running
- Look at robot's serial output for errors
- Try power cycling the robot

### Display Issues

**Map not updating:**
- Check that visualization is receiving data (updateCount should increase)
- Verify robot is sending map data
- Check console output for error messages

**Obstacles or scan points not showing:**
- Zoom out to see wider area (press `-`)
- Reset view (press `R`)
- Clear map data (press `SPACE`) to refresh

**Rendering is slow:**
- Reduce number of scan points on robot side
- Lower FPS if needed (change `frameRate(30)`)
- Close other applications

## Customization

### Change Robot IP

Edit `RobotMapVisualizer.pde`:
```java
final String ROBOT_IP = "192.168.X.X";
```

### Adjust Zoom Levels

In `MapRenderer.pde`:
```java
private final float MIN_SCALE = 50;    // Minimum zoom
private final float MAX_SCALE = 500;   // Maximum zoom
private final float DEFAULT_SCALE = 150; // Default zoom
```

### Modify Colors

In `MapRenderer.pde`:
```java
fill(0, 200, 0);  // Robot color (RGB)
fill(255, 0, 0, alpha);  // Obstacle color
fill(0, 0, 255);  // Scan point color
```

### Change Window Size

In `RobotMapVisualizer.pde`:
```java
void settings() {
  size(1200, 900);  // Width x Height
}
```

## Performance Tips

1. **Reduce Scan Point Density**: Limit frequency of ToF sensor readings on robot
2. **Optimize Update Rate**: Adjust WebSocket message frequency
3. **Zoom Appropriately**: Higher zoom levels are more computationally intensive
4. **Monitor FPS**: If below 20 FPS, reduce processing load

## File Structure

```
ProcessingMapVisualizer/
├── RobotMapVisualizer.pde    # Main sketch - WebSocket communication
├── MapRenderer.pde            # Rendering engine - drawing logic
└── README.md                  # This file
```

## Architecture

**RobotMapVisualizer.pde**
- Manages WebSocket connection to robot
- Parses incoming JSON map data
- Updates data structures
- Handles keyboard input
- Displays UI elements

**MapRenderer.pde**
- Handles 2D coordinate transformations
- Renders grid and reference points
- Draws robot, obstacles, and scan points
- Manages zoom and pan
- Provides coordinate conversion utilities

## Integration with Robot Code

The robot sends map data via WebSocket whenever:
- New obstacles are detected
- Scan points are added
- Robot position updates
- Mode changes

Ensure `MapDrawer.h` is included in robot code and map data is serialized to JSON before transmission.

## Known Limitations

- Maximum display area: Based on window size and zoom level
- Scan point limit: Processing performance depends on number of points (typically <10,000)
- Latency: Depends on WiFi connection quality
- Grid resolution: Limited by Processing canvas resolution

## Future Enhancements

- [ ] 3D map visualization
- [ ] Recording and playback
- [ ] Path history tracking
- [ ] Occupancy grid heatmap
- [ ] Export to file formats (PNG, SVG)
- [ ] Multiple robot support
- [ ] Advanced filtering options
- [ ] Performance metrics display

## Support

For issues or feature requests:
1. Check troubleshooting section
2. Review robot's serial output
3. Verify WebSocket connection
4. Check network connectivity

## License

Part of the SWARM robotics research initiative.

---

**Last Updated**: May 2026
**Version**: 1.0
**Compatibility**: Processing 4.0+
