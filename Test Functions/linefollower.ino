#define IR_R 34
#define IR_L  35
#define MOTOR_SPEED  200

// Right motor
int PWMA = 14;
int M1A  = 27;
int M1B  = 26;

// Left motor
int PWMB  = 32;
int M2A   = 25;
int M2B   = 33;

// ================================================================
void setup() {
  Serial.begin(115200);

  ledcAttach(PWMA, 7812, 8);   
  ledcAttach(PWMB,  7812, 8);

  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A,  OUTPUT);
  pinMode(M2B,  OUTPUT);

  pinMode(IR_R, INPUT);
  pinMode(IR_L,  INPUT);

  rotateMotor(0, 0);
}

// ================================================================
void loop() {
  int IR1 = digitalRead(IR_R);
  int IR2  = digitalRead(IR_L);

  Serial.printf("R=%d  L=%d\n", IR1, IR2);

  // Both sensors off line → go straight
  if (IR1 == LOW && IR2 == LOW) {
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
  // Right sensor on line → turn LEFT to correct
  else if (IR1 == HIGH && IR2 == LOW) {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
  }
  // Left sensor on line → turn RIGHT to correct
  else if (IR1 == LOW && IR2 == HIGH) {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
  }
  // Both sensors on line → stop
  else {
    rotateMotor(0, 0);
  }
}

// ================================================================
void rotateMotor(int rightSpeed, int leftSpeed) {

  // Right motor direction
  if (rightSpeed < 0) {
    digitalWrite(M1A, LOW);
    digitalWrite(M1B, HIGH);
  } else if (rightSpeed > 0) {
    digitalWrite(M1A, HIGH);
    digitalWrite(M1B, LOW);
  } else {
    digitalWrite(M1A, LOW);
    digitalWrite(M1B, LOW);
  }

  // Left motor direction
  if (leftSpeed < 0) {
    digitalWrite(M2A, LOW);
    digitalWrite(M2B, HIGH);
  } else if (leftSpeed > 0) {
    digitalWrite(M2A, HIGH);
    digitalWrite(M2B, LOW);
  } else {
    digitalWrite(M2A, LOW);
    digitalWrite(M2B, LOW);
  }

  // ESP32: ledcWrite instead of analogWrite
  ledcWrite(PWMA, abs(rightSpeed));
  ledcWrite(PWMB,  abs(leftSpeed));
}