#ifndef TOFSENSOR_H
#define TOFSENSOR_H

#include <Wire.h>
#include <VL53L0X.h>

class ToFSensor {
private:
  VL53L0X sensor;
  uint16_t distance;
  bool initialized;
  int failCount;
  
public:
  ToFSensor() : distance(0), initialized(false), failCount(0) {}

  bool begin(uint8_t sda = 21, uint8_t scl = 22) {
    Wire.begin(sda, scl);
    
    Serial.println("Starting ToF sensor initialization...");
    
    // Give the sensor time to boot
    delay(100);
    
    if (!sensor.init()) {
      Serial.println("ERROR: Failed to detect and initialize VL53L0X sensor");
      Serial.println("Check I2C connections (SDA=21, SCL=22)");
      initialized = false;
      return false;
    }
    
    Serial.println("ToF sensor detected successfully");
    
    // Configure sensor for better performance
    sensor.setTimeout(50);
    
    // Perform initial measurement to warm up
    for (int i = 0; i < 5; i++) {
      uint16_t testRead = sensor.readRangeSingleMillimeters();
      delay(10);
    }
    
    initialized = true;
    failCount = 0;
    Serial.println("ToF sensor ready - starting measurements");
    return true;
  }

  uint16_t readDistance() {
    if (!initialized) return 0;
    
    distance = sensor.readRangeSingleMillimeters();
    
    if (sensor.timeoutOccurred()) {
      failCount++;
      if (failCount % 10 == 0) {
        Serial.println("ToF timeout warning");
      }
      return 0;
    } else {
      failCount = 0;
    }
    
    return distance;
  }

  // Read multiple samples and average (for robustness during mapping)
  uint16_t readAveragedDistance(int samples = 5) {
    if (!initialized) return 0;
    
    uint32_t sum = 0;
    int validCount = 0;
    uint16_t minVal = 65535;
    uint16_t maxVal = 0;
    
    for (int i = 0; i < samples; i++) {
      uint16_t reading = sensor.readRangeSingleMillimeters();
      
      // Validate reading
      if (!sensor.timeoutOccurred() && reading > 50 && reading < 2000) {
        sum += reading;
        validCount++;
        minVal = min(minVal, reading);
        maxVal = max(maxVal, reading);
      }
      delayMicroseconds(500);  // Brief delay between samples
    }
    
    if (validCount == 0) {
      failCount++;
      return 0;
    }
    
    failCount = 0;
    
    // Return average of middle values (exclude outliers)
    if (validCount > 2) {
      sum = sum - minVal - maxVal;
      validCount = validCount - 2;
    }
    
    distance = sum / validCount;
    return distance;
  }

  uint16_t getLastDistance() {
    return distance;
  }

  bool sensorOk() {
    return !sensor.timeoutOccurred();
  }

  bool isInitialized() {
    return initialized;
  }
};

#endif
