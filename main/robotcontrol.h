
#ifndef ROBOTCONTROL_H
#define ROBOTCONTROL_H

class robotcontrol {
private:
  mpu* imu;
  PID* pid;
  MotorControl* motor;
  Debugger* dbg;
  int LS, RS;
  int basespeed = 200;
  int turnspeed = 150;

  const int wheelDiameter = 67;                             // mm
  const float wheelCircumference = 3.1416 * wheelDiameter;  // mm
  int estimatedRPM = 114;
public:
  robotcontrol(mpu* imu,
               PID* pid,
               MotorControl* motor,
               Debugger* dbg)
    : imu(imu), pid(pid), motor(motor), dbg(dbg) {}

  void moverobot(char dir, long time) {
    motor->move(dir);
    unsigned long start = millis();
    while (millis() - start < time) {
      imu->update();
      float yawrate = imu->getz();
      float correction = pid->compute(yawrate, 0);

      if (dir == 'f') {
        LS = constrain(basespeed + correction, 0, 255);
        RS = constrain(basespeed - correction, 0, 255);
      } else {
        LS = constrain(basespeed - correction, 0, 255);
        RS = constrain(basespeed + correction, 0, 255);
      }
      motor->setPWM(LS, RS);
    }
    motor->move('s');
  }

  void turn(float targetangle) {
    imu->resetAngle();  // Reset Kalman filter angle to 0
    unsigned long startTime = millis();
    float tolerance = 2.0;  // Tolerance in degrees
    
    while (true) {
      imu->update();
      float currentAngle = imu->getFilteredAngle();  // Get Kalman-filtered angle
      
      // Check if we've reached target angle
      if (abs(targetangle - currentAngle) < tolerance) {
        // Verify stability - take multiple readings to ensure we're actually there
        int stableCount = 0;
        for (int i = 0; i < 5; i++) {
          imu->update();
          currentAngle = imu->getFilteredAngle();
          if (abs(targetangle - currentAngle) < tolerance) {
            stableCount++;
          }
          delay(10);
        }
        if (stableCount >= 4) break;  // 4 out of 5 readings are within tolerance
      }
      
      // Timeout after 5 seconds
      if (millis() - startTime > 5000) {
        Serial.println("Turn timeout!");
        break;
      }
      
      // Determine turn direction and set motor speed with PID correction
      float yawrate = imu->getz();
      float pidCorrection = pid->compute(yawrate, 0);
      
      int leftMotor = turnspeed;
      int rightMotor = turnspeed;
      
      if (currentAngle < targetangle) {
        // Need to turn left more
        motor->move('l');
        leftMotor = constrain(turnspeed - pidCorrection, 50, 255);
        rightMotor = constrain(turnspeed + pidCorrection, 50, 255);
      } else {
        // Need to turn right more
        motor->move('r');
        rightMotor = constrain(turnspeed - pidCorrection, 50, 255);
        leftMotor = constrain(turnspeed + pidCorrection, 50, 255);
      }
      
      motor->setPWM(leftMotor, rightMotor);
      delay(10);
    }
    
    motor->move('s');
    pid->reset();
  }

  void start_calibration() {
    moverobot('f', 1000);
  }

  void calibrateRPM(float distanceCM) {
    float distanceMM = distanceCM * 10;
    estimatedRPM = (distanceMM / wheelCircumference) / (1000 / 60000.0);

    Serial.print("Updated Estimated RPM: ");
    Serial.println(estimatedRPM);
  }

  void moveED(float distanceCM) {
    float distanceMM = distanceCM * 10;
    float numRevolutions = distanceMM / wheelCircumference;
    float timePerRev = 60.0 / estimatedRPM;
    float totalTimeSec = numRevolutions * timePerRev;
    unsigned long totalTimeMS = totalTimeSec * 1000;

    moverobot('f',totalTimeMS);
  }

};


#endif
