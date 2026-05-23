#include "MPU.h"
#include <Wire.h>
#include "PID.h"

#include "motorcontrol.h"
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include "debugger.h"
#include "webhandler.h"
#include "robotcontrol.h"
#include "ToFSensor.h"
#include "ToFMap.h"

PID p(1.0, 0.1, 0.1); //As per hardware testing, these values work well for the robot's response. Adjust as needed based on performance.
mpu m;
ToFSensor tofSensor;
ToFMap envMap;

// Mode tracking variables
bool lineFollowingMode = false;
bool obstacleAvoidanceMode = false;

// Motor pins (example pins, change as needed)
const int M1A = 27;
const int M1B = 26;
const int M2A = 25;
const int M2B = 33;
const int PWMA = 14;
const int PWMB = 32;

const char* ssid = "StochBot";
const char* password = "12345678";


MotorControl motor(M1A, M1B, M2A, M2B, PWMA, PWMB);
WebSocketsServer socket = WebSocketsServer(81);
Debugger debugger(&socket);
robotcontrol robot(&m,&p,&motor,&debugger);
WebHandler web(&motor, &socket, &debugger, &robot, &tofSensor, &envMap);



void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Start WiFi in AP mode

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("WiFi AP started. SSID: ");
  Serial.println(WiFi.softAPIP());

  if (m.begin()) {
    Serial.println("MPU CONNECTED");
    m.calibrate();
  } else {
    Serial.println("MPU NOT CONNECTED");
  }
  
  // Initialize ToF sensor
  if (tofSensor.begin()) {
    Serial.println("ToF sensor ready for mapping");
  } else {
    Serial.println("ToF sensor initialization failed");
  }
  
  motor.begin();
  // motor.test(); // Optionally comment out after testing
  web.begin();
  
}


void loop() {
  // if (m.check()) {
  //   m.update();
  // } else {
  //   Serial.println("MPU Failed to Update");
  // }
  // if (m.check()) {
  //   Serial.println(p.compute(m.getz()));
  // } else {
  //   Serial.println("MPU Failed to get Value");
  // }
  web.update();
  
  // Read ToF sensor and update map
  if (tofSensor.isInitialized()) {
    static unsigned long lastMapUpdate = 0;
    static int scanAngle = 0;
    static int readCount = 0;
    
    uint16_t distance = tofSensor.readDistance();
    
    // Update robot angle from MPU if available
    if (m.check()) {
      m.update();
      envMap.setRobotAngle(m.getFilteredAngle());
    }
    
    // Debug: Print sensor readings every 30 measurements
    if (readCount++ % 30 == 0 && readCount > 0) {
      Serial.print("ToF angle: ");
      Serial.print(scanAngle);
      Serial.print("° distance: ");
      Serial.print(distance);
      Serial.println("mm");
    }
    
    // Add measurement to map - be strict about valid readings
    if (tofSensor.sensorOk() && distance > 50 && distance < 2000) {  // Valid range: 50mm to 2000mm
      envMap.addMeasurement(scanAngle, distance);
      
      if (readCount % 72 == 0) {  // Every 72 readings (360 degrees)
        Serial.println("Map scan cycle complete");
      }
    }
    
    scanAngle = (scanAngle + 5) % 360;  // 5 degree increments, wrap at 360
    
    // Send map update every 500ms with obstacle count
    if (millis() - lastMapUpdate > 500) {
      lastMapUpdate = millis();
      String mapJSON = envMap.getCompactMapJSON();
      
      // Debug: Print map update info
      Serial.print("Sending map JSON (");
      Serial.print(mapJSON.length());
      Serial.println(" bytes)");
      
      // Send to all connected clients
      for (uint8_t i = 0; i < socket.connectedClients(); i++) {
        socket.sendTXT(i, mapJSON);
      }
    }
  } else {
    // ToF not initialized - try again
    static unsigned long lastRetry = 0;
    if (millis() - lastRetry > 2000) {
      lastRetry = millis();
      if (tofSensor.begin()) {
        Serial.println("ToF sensor reconnected");
      }
    }
  }
}
