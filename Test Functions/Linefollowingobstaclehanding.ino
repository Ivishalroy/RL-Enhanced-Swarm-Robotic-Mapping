#include <ESP32Servo.h>

#define IR_R        34
#define IR_L        35
#define TRIG_PIN     4
#define ECHO_PIN    15
#define SERVO_PIN   13
#define MOTOR_SPEED 180

// Right motor
int PWMA = 14;
int M1A  = 27;
int M1B  = 26;
// Left motor
int PWMB = 32;
int M2A  = 25;
int M2B  = 33;

Servo myservo;
long duration;
int  distance;

// ── Forward declarations ──────────────────────────────────────
int  measureDistance();
void rotateMotor(int rightSpeed, int leftSpeed);
void avoidObstacle();

// ================================================================
void setup() {
  Serial.begin(115200);

  ledcAttach(PWMA, 7812, 8);
  ledcAttach(PWMB, 7812, 8);

  myservo.attach(SERVO_PIN);
  myservo.write(90);

  pinMode(M1A,      OUTPUT);
  pinMode(M1B,      OUTPUT);
  pinMode(M2A,      OUTPUT);
  pinMode(M2B,      OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_R,     INPUT);
  pinMode(IR_L,     INPUT);

  rotateMotor(0, 0);
}

// ================================================================
void loop() {
  delay(50);
  distance = measureDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 10) {
    rotateMotor(0, 0);
    delay(200);
    avoidObstacle();   // ✅ Full avoidance routine
    return;
  }

  // ── Line following ────────────────────────────────────────────
  int IR1 = digitalRead(IR_R);
  int IR2  = digitalRead(IR_L);
  Serial.printf("R=%d  L=%d\n", IR1, IR2);

  if      (IR1 == LOW  && IR2 == LOW)  rotateMotor(MOTOR_SPEED,  MOTOR_SPEED);
  else if (IR1 == HIGH && IR2 == LOW)  rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
  else if (IR1 == LOW  && IR2 == HIGH) rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
  else                                 rotateMotor(0, 0);
}

// ================================================================
void avoidObstacle() {
  Serial.println("=== Obstacle Avoidance START ===");

  // ── Step 1: Move backward for 300 ms ─────────────────────────
  Serial.println("Step 1: Reversing...");
  rotateMotor(-MOTOR_SPEED, -MOTOR_SPEED);
  delay(300);
  rotateMotor(0, 0);
  delay(150);

  // ── Step 2: Servo scans RIGHT (180°) then LEFT (0°) ──────────
  Serial.println("Step 2: Scanning right...");
  myservo.write(180);
  delay(600);
  int distRight = measureDistance();
  Serial.print("Right distance: ");
  Serial.println(distRight);

  Serial.println("Step 2: Scanning left...");
  myservo.write(0);
  delay(600);
  int distLeft = measureDistance();
  Serial.print("Left distance: ");
  Serial.println(distLeft);

  // Servo back to center
  myservo.write(90);
  delay(300);

  // ── Step 3: Decide clear side ─────────────────────────────────
  bool turnRight = (distRight >= distLeft);   // pick the side with more space
  Serial.print("Turning: ");
  Serial.println(turnRight ? "RIGHT" : "LEFT");

  // ── Step 4: Turn toward clear side, move 300 ms ───────────────
  Serial.println("Step 4: Turning toward clear side...");
  if (turnRight) {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);   // turn right in place
  } else {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);   // turn left in place
  }
  delay(300);
  rotateMotor(0, 0);
  delay(150);

  Serial.println("Step 4: Moving forward 300 ms on clear side...");
  rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  delay(300);
  rotateMotor(0, 0);
  delay(150);

  // ── Step 5: Turn opposite side, move 300 ms ───────────────────
  Serial.println("Step 5: Turning to opposite side...");
  if (turnRight) {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);   // now turn left
  } else {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);   // now turn right
  }
  delay(300);
  rotateMotor(0, 0);
  delay(150);

  Serial.println("Step 5: Moving forward 300 ms...");
  rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  delay(300);
  rotateMotor(0, 0);
  delay(150);

  // ── Step 6: Turn back to original direction ───────────────────
  Serial.println("Step 6: Re-aligning to original direction...");
  if (turnRight) {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);   // turn left to re-align
  } else {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);   // turn right to re-align
  }
  delay(300);
  rotateMotor(0, 0);
  delay(150);

  // ── Step 7: Final distance check before resuming ─────────────
  Serial.println("Step 7: Final distance check...");
  int finalDist = measureDistance();
  Serial.print("Final distance: ");
  Serial.println(finalDist);

  if (finalDist < 10) {
    Serial.println("Still blocked! Retrying avoidance...");
    avoidObstacle();   // ✅ Recursive retry if still blocked
  } else {
    Serial.println("=== Path clear! Resuming line following ===");
  }
}

// ================================================================
int measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long dur = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (dur == 0) return 999;
  return (int)(dur * 0.034 / 2);
}

// ================================================================
void rotateMotor(int rightSpeed, int leftSpeed) {

  if      (rightSpeed < 0) { digitalWrite(M1A, LOW);  digitalWrite(M1B, HIGH); }
  else if (rightSpeed > 0) { digitalWrite(M1A, HIGH); digitalWrite(M1B, LOW);  }
  else                     { digitalWrite(M1A, LOW);  digitalWrite(M1B, LOW);  }

  if      (leftSpeed < 0)  { digitalWrite(M2A, LOW);  digitalWrite(M2B, HIGH); }
  else if (leftSpeed > 0)  { digitalWrite(M2A, HIGH); digitalWrite(M2B, LOW);  }
  else                     { digitalWrite(M2A, LOW);  digitalWrite(M2B, LOW);  }

  ledcWrite(PWMA, abs(rightSpeed));
  ledcWrite(PWMB, abs(leftSpeed));
}