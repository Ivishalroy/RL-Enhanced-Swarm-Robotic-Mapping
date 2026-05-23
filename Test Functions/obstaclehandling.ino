#include <ESP32Servo.h>

// ── Pin Definitions ──────────────────────────────────────────
#define EN_R      14
#define IN1       27
#define IN2       26

#define EN_L      32
#define IN3       25
#define IN4       33

#define TRIG_PIN   4
#define ECHO_PIN  15
#define SERVO_PIN 13
#define MOTOR_SPEED  100

Servo myservo;

long duration;
int  distance;
int  servoReadLeft  = 0;
int  servoReadRight = 0;

// ── Forward declarations ──────────────────────────────────────
int  measureDistance();
void stopCar();
void moveForwardCar();
void moveBackwardCar();
void turnRightCar();
void turnLeftCar();

// ─────────────────────────────────────────────────────────────
void setup() {
  delay(1000);
  Serial.begin(115200);

  myservo.attach(SERVO_PIN);
  myservo.write(90);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(EN_R, OUTPUT);
  pinMode(EN_L, OUTPUT);
  digitalWrite(EN_R, HIGH);
  digitalWrite(EN_L, HIGH);

  Serial.println("Robot initialized.");
}

// ─────────────────────────────────────────────────────────────
void loop() {
  delay(50);
  distance = measureDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 13) {
    Serial.println("Obstacle detected! Stopping...");
    stopCar();
    delay(300);

    // Reverse away from obstacle
    Serial.println("Reversing...");
    moveBackwardCar();
    delay(300);
    stopCar();
    delay(300);

    // Scan left and right to find clear path
    Serial.println("Scanning right...");
    myservo.write(0);
    delay(600);
    servoReadRight = measureDistance();
    Serial.print("Right distance: "); Serial.println(servoReadRight);

    Serial.println("Scanning left...");
    myservo.write(180);
    delay(600);
    servoReadLeft = measureDistance();
    Serial.print("Left distance: "); Serial.println(servoReadLeft);

    // Re-centre servo
    myservo.write(90);
    delay(400);

    // Turn toward the clearer side
    if (servoReadLeft > servoReadRight) {
      Serial.println("Turning left — more space on left");
      turnLeftCar();
    } else {
      Serial.println("Turning right — more space on right");
      turnRightCar();
    }
  } else {
    Serial.println("Path clear — moving forward");
    moveForwardCar();
  }
}

// ── Motion helpers ────────────────────────────────────────────
void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void moveForwardCar() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackwardCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeftCar() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   
  delay(300);
  stopCar();
}

void turnRightCar() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  
  delay(300);
  stopCar();
}

// ── Ultrasonic measurement ────────────────────────────────────
int measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  return (int)(duration * 0.034 / 2);
}