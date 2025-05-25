#include <Servo.h>

Servo servoR, servoL;

// Analog input pins for joystick
const int potX = A0;
const int potY = A1;

// Relay and PWM pins
const int relayR = 53;
const int relayL = 51;
const int pwmR = 8;
const int pwmL = 9;

// Control and indicator pins
const int buttonPin = 4;
const int ledRed = 11;
const int ledGreen = 12;
const int lockR = 24;
const int lockL = 22;

int initX = 0, initY = 0;
int dem = 0, dem2 = 0;
bool positionCaptured = false;

void setup() {
  servoR.attach(pwmR);
  servoL.attach(pwmL);
  Serial.begin(9600);

  pinMode(potX, INPUT);
  pinMode(potY, INPUT);
  pinMode(relayR, OUTPUT);
  pinMode(relayL, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(lockR, OUTPUT);
  pinMode(lockL, OUTPUT);
  pinMode(2, INPUT_PULLUP); // Interrupt pin

  attachInterrupt(0, changeMode, LOW);

  // Initialize servos and relays
  servoR.write(0);
  servoL.write(0);
  digitalWrite(relayR, HIGH);
  digitalWrite(relayL, HIGH);
}

void loop() {
  dem2 = 0;

  if (dem == 0) {
    idleMode();
    return;
  }

  activeMode();
}

// ISR: Handle mode change button press
void changeMode() {
  if (digitalRead(buttonPin) == LOW) {
    delay(500);
    if (++dem2 >= 10) {
      dem2 = 0;
      dem = (dem + 1) % 2;

      digitalWrite(ledRed, dem == 0);
      digitalWrite(ledGreen, dem == 1);

      Serial.println("Mode changed");
    }
  }
}

// Mode 0: Idle state (LED red on, locks open)
void idleMode() {
  digitalWrite(ledRed, HIGH);
  digitalWrite(lockR, LOW);
  digitalWrite(lockL, LOW);
  digitalWrite(ledGreen, LOW);
  positionCaptured = false;
}

// Mode 1: Control vehicle with joystick
void activeMode() {
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(lockR, HIGH);
  digitalWrite(lockL, HIGH);

  int xNow = analogRead(potX);
  int yNow = analogRead(potY);

  if (!positionCaptured) {
    delay(1000); // Allow analog readings to stabilize
    initX = xNow;
    initY = yNow;
    positionCaptured = true;
    Serial.print("Init X: "); Serial.println(initX);
    Serial.print("Init Y: "); Serial.println(initY);
  }

  handleMovement(xNow, yNow);
}

// Handle movement directions based on joystick input
void handleMovement(int xNow, int yNow) {
  int deltaX = xNow - initX;
  int deltaY = yNow - initY;

  if (abs(deltaX) < 100 && abs(deltaY) < 100) {
    stopVehicle();
    return;
  }

  if (deltaX < -300 && abs(deltaY) < 250) forward();
  else if (deltaX > 300 && abs(deltaY) < 250) backward();
  else if (deltaY < -300 && abs(deltaX) < 200) rotateRight();
  else if (deltaY > 300 && abs(deltaX) < 200) rotateLeft();
  else if (deltaX < -300 && deltaY < -250) forwardRight();
  else if (deltaX < -300 && deltaY > 250) forwardLeft();
  else if (deltaX > 250 && deltaY > 250) backwardRight();
  else if (deltaX > 250 && deltaY < -250) backwardLeft();
}

// Stop vehicle
void stopVehicle() {
  servoR.write(0);
  servoL.write(0);
  Serial.println("Stopped");
}

// Move forward
void forward() {
  digitalWrite(relayR, HIGH);
  digitalWrite(relayL, HIGH);
  int posR = map(analogRead(potX), initX - 70, 0, 0, 135);
  int posL = map(analogRead(potX), initX - 70, 0, 0, 67);
  servoR.write(posR);
  servoL.write(posL);
  Serial.println("Forward");
}

// Move backward
void backward() {
  digitalWrite(relayR, LOW);
  digitalWrite(relayL, LOW);
  servoR.write(50);
  servoL.write(55);
  Serial.println("Backward");
}

// Rotate right in place
void rotateRight() {
  digitalWrite(relayR, LOW);
  digitalWrite(relayL, HIGH);
  servoR.write(90);
  servoL.write(63);
  Serial.println("Rotate right");
}

// Rotate left in place
void rotateLeft() {
  digitalWrite(relayR, HIGH);
  digitalWrite(relayL, LOW);
  servoR.write(90);
  servoL.write(63);
  Serial.println("Rotate left");
}

// Forward right turn
void forwardRight() {
  digitalWrite(relayR, HIGH);
  digitalWrite(relayL, HIGH);
  servoR.write(90);
  int posL = map(analogRead(potY), initY - 80, 0, 81, 108);
  servoL.write(posL);
  Serial.println("Forward right");
}

// Forward left turn
void forwardLeft() {
  digitalWrite(relayR, HIGH);
  digitalWrite(relayL, HIGH);
  int posR = map(analogRead(potY), initY + 80, 1023, 72, 153);
  servoR.write(posR);
  servoL.write(70);
  Serial.println("Forward left");
}

// Backward right turn
void backwardRight() {
  digitalWrite(relayR, LOW);
  digitalWrite(relayL, LOW);
  servoR.write(60);
  int posL = map(analogRead(potY), initY + 80, 1023, 60, 90);
  servoL.write(posL);
  Serial.println("Backward right");
}

// Backward left turn
void backwardLeft() {
  digitalWrite(relayR, LOW);
  digitalWrite(relayL, LOW);
  int posR = map(analogRead(potY), initY - 80, 0, 60, 120);
  servoR.write(posR);
  servoL.write(70);
  Serial.println("Backward left");
}
