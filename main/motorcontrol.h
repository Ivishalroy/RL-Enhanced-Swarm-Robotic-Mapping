// motorcontrol.h
// Abstraction control library for ESP32 car motor control
// Controls speed and direction of two motors

#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

class MotorControl {
  public:
    MotorControl(int m1a, int m1b, int m2a, int m2b, int pwmA, int pwmB) {
      _m1a = m1a;
      _m1b = m1b;
      _m2a = m2a;
      _m2b = m2b;
      _pwmA = pwmA;
      _pwmB = pwmB;
    }

    void begin() {
      pinMode(_m1a, OUTPUT);
      pinMode(_m1b, OUTPUT);
      pinMode(_m2a, OUTPUT);
      pinMode(_m2b, OUTPUT);
      pinMode(_pwmA, OUTPUT);
      pinMode(_pwmB, OUTPUT);
      digitalWrite(_m1a, LOW);
      digitalWrite(_m1b, LOW);
      digitalWrite(_m2a, LOW);
      digitalWrite(_m2b, LOW);
      analogWrite(_pwmA, baseSpeed);
      analogWrite(_pwmB, baseSpeed);
    }

    void setPWM(int left, int right) {
      analogWrite(_pwmA, left);
      analogWrite(_pwmB, right);
    }

    void move(char dir) {
      switch (dir) {
        case 'f': forward(); break;
        case 'b': backward(); break;
        case 'l': left(); break;
        case 'r': right(); break;
        case 's': stop(); break;
        default: stop(); break;
      }
    }

    void forward() {
      digitalWrite(_m1a, HIGH);
      digitalWrite(_m1b, LOW);
      digitalWrite(_m2a, HIGH);
      digitalWrite(_m2b, LOW);
    }

    void backward() {
      digitalWrite(_m1a, LOW);
      digitalWrite(_m1b, HIGH);
      digitalWrite(_m2a, LOW);
      digitalWrite(_m2b, HIGH);
    }

    void left() {
      digitalWrite(_m1a, LOW);
      digitalWrite(_m1b, HIGH);
      digitalWrite(_m2a, HIGH);
      digitalWrite(_m2b, LOW);
    }

    void right() {
      digitalWrite(_m1a, HIGH);
      digitalWrite(_m1b, LOW);
      digitalWrite(_m2a, LOW);
      digitalWrite(_m2b, HIGH);
    }

    void stop() {
      digitalWrite(_m1a, LOW);
      digitalWrite(_m1b, LOW);
      digitalWrite(_m2a, LOW);
      digitalWrite(_m2b, LOW);
    }

    void test() {
      forward();
      delay(1000);
      backward();
      delay(1000);
      left();
      delay(1000);
      right();
      delay(1000);
      stop();
    }

  private:
    int _m1a, _m1b, _m2a, _m2b, _pwmA, _pwmB;
    int baseSpeed =200;
};

#endif // MOTORCONTROL_H