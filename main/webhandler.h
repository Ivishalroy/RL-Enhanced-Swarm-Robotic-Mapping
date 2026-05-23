// webhandler.h
// Handles WebServer, WebSocketsServer, and command parsing for ESP32 car

#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "motorcontrol.h"
#include "debugger.h"
#include "Interface.h"
#include "robotcontrol.h"


class WebHandler {
public:
  WebHandler(MotorControl* motor, WebSocketsServer* socket, Debugger* debugger, robotcontrol* robot, void* tofSensor = nullptr, void* map = nullptr)
    : _motor(motor), _socket(socket), _debugger(debugger), robot(robot), _tofSensor(tofSensor), _map(map), _lineFollowingMode(nullptr), _obstacleAvoidanceMode(nullptr) {}

  void setModePointers(bool* lineFollowMode, bool* obstacleAvoidMode) {
    _lineFollowingMode = lineFollowMode;
    _obstacleAvoidanceMode = obstacleAvoidMode;
  }

  void begin() {

    server.on("/", [this]() {
      server.send(200, "text/html", MAIN_page);
    });
    server.begin();
    server.begin();

    _socket->begin();
    _socket->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
      this->onWebSocketEvent(num, type, payload, length);
    });
  }

  void update() {
    server.handleClient();
    _socket->loop();
  }

  // Send map update via WebSocket
  void sendMapUpdate(void* mapPtr) {
    if (!mapPtr || !_socket) return;
    
    // Cast to ToFMap* (avoiding circular include)
    // Send as string since we can't directly access ToFMap here
    String mapJSON = "{\"type\":\"map\",\"data\":{}}";
    
    // For now, send a placeholder - the actual map sending happens in robotcontrol
    for (uint8_t i = 0; i < _socket->connectedClients(); i++) {
      _socket->sendTXT(i, mapJSON);
    }
  }

  void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
      DynamicJsonDocument doc(256);
      DeserializationError err = deserializeJson(doc, payload, length);

      if (!err) {
        String key = doc["key"] | "";
        String value = doc["value"] | "";
        Serial.println(key);
        Serial.println(value);
        
        // Handle mode toggles
        if (key == "lineFollow") {
          if (_lineFollowingMode) {
            *_lineFollowingMode = (value == "on");
            _debugger->sendDebug("Line Following: " + value);
          }
        } else if (key == "obstacleAvoid") {
          if (_obstacleAvoidanceMode) {
            *_obstacleAvoidanceMode = (value == "on");
            _debugger->sendDebug("Obstacle Avoidance: " + value);
          }
        } else if (key == "move" && _motor) {
          _motor->move(value[0]);
          _debugger->sendDebug("Move: " + value);
        }else if(key == "fs" || key == "bs"){
          float val = doc["value"].as<float>();
          robot->moverobot(key[0], val);
        }else if (key == "turn"){
          float val = doc["value"].as<float>();
          robot->turn(val);
        }else if (key == "start_calibration"){
          robot->start_calibration();
        }else if (key == "calibrate"){
          float val = doc["value"].as<float>();
          robot->calibrateRPM(val);
        }else if(key == "moveED"){
          float val = doc["value"].as<float>();
          robot->moveED(val);
        }
      }
    }
  }

private:
  MotorControl* _motor;
  WebSocketsServer* _socket;
  Debugger* _debugger;
  WebServer server{ 80 };
  robotcontrol* robot;
  void* _tofSensor;
  void* _map;
  bool* _lineFollowingMode;
  bool* _obstacleAvoidanceMode;
};

#endif  // WEBHANDLER_H