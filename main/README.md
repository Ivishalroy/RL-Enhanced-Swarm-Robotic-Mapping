# SWARM Robot Project

## Overview
This project is an autonomous robot system built on an ESP32 microcontroller that combines real-time sensor mapping, motor control, and web-based communication. The robot is capable of autonomous navigation, obstacle avoidance, and environmental mapping using Time-of-Flight (ToF) sensors.

## Features
- **WiFi Communication**: AP mode WiFi connectivity with real-time WebSocket communication
- **Environmental Mapping**: ToF sensor-based mapping to detect obstacles and create environment representation
- **Motor Control**: Dual-motor control system with PWM support
- **Inertial Measurement**: MPU (IMU) sensor integration for orientation and angle tracking
- **PID Control**: Feedback control for precise motor response
- **Line Following**: Automatic line detection and following capability
- **Obstacle Avoidance**: Real-time obstacle detection and avoidance
- **Web Interface**: WebSocket-based remote control and monitoring
- **Kalman Filtering**: Advanced sensor fusion for accurate orientation estimation

## Hardware Components
- **Microcontroller**: ESP32
- **Motion Sensor**: MPU6050/MPU9250 (Inertial Measurement Unit)
- **Distance Sensor**: VL53L0X Time-of-Flight (ToF) Sensor
- **Motor Driver**: Dual motor control with PWM pins
- **WiFi**: Built-in WiFi module

## Pin Configuration
```
Motor Pins:
- M1A: GPIO 27
- M1B: GPIO 26
- M2A: GPIO 25
- M2B: GPIO 33
- PWMA: GPIO 14
- PWMB: GPIO 32
```

## WiFi Configuration
- **SSID**: StochBot
- **Password**: 12345678
- **Mode**: Access Point (AP)
- **WebSocket Port**: 81

## Project Structure

### Core Components

#### `main.ino`
Main sketch file containing setup and control loop. Manages sensor updates, motor control, and WiFi communication.

#### `motorcontrol.h`
Motor control abstraction layer for managing dual DC motors with PWM control.

#### `robotcontrol.h`
High-level robot control logic, including motion primitives and autonomous behavior.

#### `MPU.h`
Interface for the MPU IMU sensor. Handles initialization, calibration, and orientation tracking with Kalman filtering.

#### `PID.h`
PID controller implementation for closed-loop motor speed and direction control.

#### `ToFSensor.h`
Time-of-Flight sensor driver for distance measurement.

#### `ToFMap.h`
Environmental mapping system using ToF sensor data to build a representation of surroundings.

#### `webhandler.h`
WebSocket and HTTP server management for remote communication and control.

#### `robotcontrol.h`
Robot behavior control including line following and obstacle avoidance modes.

#### `KalmanFilter.h`
Kalman filtering algorithm for sensor fusion and orientation estimation.

#### `debugger.h`
Debugging and logging utilities for real-time diagnostics.

#### `Interface.h`
Communication interface definitions.

## Getting Started

### Prerequisites
- Arduino IDE with ESP32 board support
- Required libraries:
  - Wire (I2C communication)
  - WebServer (ESP32 built-in)
  - WebSocketsServer
  - WiFi (ESP32 built-in)

### Installation & Upload
1. Connect ESP32 via USB
2. Open `main.ino` in Arduino IDE
3. Select appropriate board and COM port
4. Upload sketch

### Operation
1. Power on the robot
2. Connect to WiFi network "StochBot"
3. Access web interface at robot's IP address
4. Use control interface to command robot movement and monitor sensor data
5. Environmental maps are streamed via WebSocket to connected clients

## Communication Protocol
- **WebSocket**: Real-time bidirectional communication with web clients
- **Mapping Data**: ToF sensor readings and map data sent via WebSocket
- **Control Commands**: Motor and behavior control via WebSocket messages

## Autonomous Modes
- **Line Following**: Detects and follows dark lines
- **Obstacle Avoidance**: Detects obstacles and navigates around them
- **Mapping**: Continuously scans environment and builds map representation

## Python Map Visualization
For map visualization and analysis, use the included Python mapping interface. See `MapDrawer.h` for integration details.

## Performance Tuning
- **PID Parameters**: Located in `main.ino`. Adjust (1.0, 0.1, 0.1) based on observed performance
- **Motor Calibration**: Run `motor.test()` to verify motor response
- **Sensor Calibration**: MPU requires calibration at startup (automatic in setup)

## Troubleshooting
- **MPU Not Detected**: Check I2C connections and verify Wire library initialization
- **ToF Sensor Issues**: Ensure VL53L0X is properly connected via I2C
- **WiFi Connection**: Verify SSID and password configuration
- **Motor Not Responding**: Check motor pin connections and power supply

## License
This project is part of the SWARM robotics research initiative.

## Contributors
- Project team members and collaborators

## Support
For issues and questions, please refer to the project documentation or contact the development team.
