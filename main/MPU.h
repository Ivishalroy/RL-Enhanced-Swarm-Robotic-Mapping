
#ifndef MPU_H
#define MPU_H

#include <MPU6050.h>
#include "KalmanFilter.h"

class mpu {
public:

  bool begin() {
    mpu.initialize();
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    kalmanFilter.reset(0.0);
    return mpu.testConnection();
  }

  void update() {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    // Raw gyroscope value in deg/s
    gyroz = (float)gz / 65.5;
    yawrate = gyroz - calibrateoffset;
    
    // Calculate time delta
    unsigned long now = millis();
    if (lastUpdateTime == 0) {
      lastUpdateTime = now;
      dt = 0.01; // Default 10ms
    } else {
      dt = (now - lastUpdateTime) / 1000.0;
      lastUpdateTime = now;
    }
    
    // Update Kalman filter with gyroscope data
    kalmanFilter.updateGyro(yawrate, dt);
    
    // Optional: Use accelerometer for reference (Z-axis tilt detection)
    // For 2D rotation, accelerometer mainly provides gravity reference
    // We primarily rely on gyroscope for yaw measurement
  }

  float getz() {
    return yawrate;
  }

  // Get Kalman filtered yaw angle (accumulated rotation in degrees)
  float getFilteredAngle() {
    return kalmanFilter.getAngle();
  }

  bool check() {
    if (mpu.testConnection()) {
      return true;
    } else {
      return false;
    }
  }

  void calibrate() {
    float sum = 0.0;
    for (int i = 0; i < sample; i++) {
      int16_t gx, gy, gz;
      mpu.getRotation(&gx, &gy, &gz);
      sum += (float)gz / 65.5;  // deg/s for Z axis
      delay(1);
    }
    calibrateoffset = sum/sample;
  }

  void resetAngle() {
    kalmanFilter.reset(0.0);
  }

private:
  MPU6050 mpu;
  KalmanFilter kalmanFilter;
  float gyroz;
  float calibrateoffset = 0.0;
  int sample = 2000;
  float yawrate = 0.0;
  float dt = 0.0;
  unsigned long lastUpdateTime = 0;
};

#endif
