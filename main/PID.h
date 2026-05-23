
#ifndef PID_H
#define PID_H

class PID {
public:
  float kp, kd, ki;
  float error, derivative, integral, previouserror;

  PID(float p, float i, float d) {
    setconstant(p, i, d);
    reset();
  }

  void setconstant(float p, float i, float d) {
    if (p < 0 || i < 0 || d < 0) {
      kp = 1.0;
      kd = 1.0;
      ki = 1.0;
    }
    kp = p;
    kd = d;
    ki = i;
  }

  float compute(float input, float target = 0) {
    error = target - input;
    integral += error;
    derivative = error - previouserror;
    previouserror = error;

    return kp * error + ki * integral + kd * derivative;
  }

  void reset() {
    error = derivative = integral = previouserror = 0;
  }

private:
};

#endif
