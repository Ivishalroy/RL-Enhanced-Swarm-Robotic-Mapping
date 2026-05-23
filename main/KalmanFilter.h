#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

class KalmanFilter {
private:
  // State variables
  float angle;           // Estimated angle (degrees)
  float angleError;      // Estimate error
  float gyroError;       // Gyro error (process noise)
  float accelError;      // Accel error (measurement noise)
  
  // Kalman gain
  float kalmanGain;

public:
  KalmanFilter(float initAngle = 0.0, float processNoise = 0.01, float measurementNoise = 0.5) {
    angle = initAngle;
    angleError = 1.0;
    gyroError = processNoise;      // Process noise - lower = trust gyro more
    accelError = measurementNoise; // Measurement noise - lower = trust accel more
    kalmanGain = 0.0;
  }

  // Update filter with gyroscope reading and time delta in seconds
  float updateGyro(float gyroRate, float dt) {
    // Predict step: estimate angle change from gyroscope
    angle += gyroRate * dt;
    
    // Update estimate error (increases due to process noise)
    angleError += gyroError;
    
    // Calculate Kalman gain
    kalmanGain = angleError / (angleError + accelError);
    
    return angle;
  }

  // Correct estimate with accelerometer measurement
  void updateAccel(float accelMeasurement) {
    // Measurement residual
    float innovation = accelMeasurement - angle;
    
    // Update angle estimate
    angle += kalmanGain * innovation;
    
    // Update estimate error (decreases due to correction)
    angleError = (1.0 - kalmanGain) * angleError;
  }

  // Get current filtered angle
  float getAngle() {
    return angle;
  }

  // Reset filter
  void reset(float initAngle = 0.0) {
    angle = initAngle;
    angleError = 1.0;
    kalmanGain = 0.0;
  }

  // Set noise parameters for tuning
  void setNoiseParameters(float processNoise, float measurementNoise) {
    gyroError = processNoise;
    accelError = measurementNoise;
  }
};

#endif
