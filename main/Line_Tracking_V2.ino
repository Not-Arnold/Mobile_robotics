// ============================================================
// Line_Tracking_V2.ino — Sensor Reading, PID Control & Motor Drive
// Reads the 5-channel IR sensor array, computes a line-position
// error, applies PID to set motor speeds, and drives the servo.
// Also contains ultrasonic obstacle detection and the celebratory
// dancing sequence triggered at the end of a delivery run.
// ============================================================

#include "config.h"

// -------------------------------------------------------
// calculateError — Map IR sensor readings to a signed error
//
// Returns:
//   -5  far left of line (sensor 0 active)
//   -1  slightly left    (sensor 1 active)
//    0  centred          (sensor 2 active)
//    1  slightly right   (sensor 3 active)
//    5  far right        (sensor 4 active)
//  100  ALL/MOST sensors active → robot is on a junction node
//   99  NO sensors active → line lost
// -------------------------------------------------------
int calculateError() {
  int threshold = 500;  // ADC readings below this value = line detected

  // Read all five IR sensors into the shared AnalogValue[] array
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
  }

  // Convert raw ADC to boolean line-detected flags
  bool s0 = AnalogValue[0] < threshold;
  bool s1 = AnalogValue[1] < threshold;
  bool s2 = AnalogValue[2] < threshold;
  bool s3 = AnalogValue[3] < threshold;
  bool s4 = AnalogValue[4] < threshold;

  // Junction detection: 3 or more adjacent sensors all see the line.
  // This typically happens when the robot reaches a node marker.
  if ((s0 && s1 && s2 && s3 && s4) || (s1 && s2 && s3 && s4) || (s0 && s1 && s2 && s3) || (s1 && s2 && s3)){
    return 100; 
  }

  // Single-sensor priority: outermost sensor takes priority
  // to give the strongest correction before veering off track.
  if (s0) return -5;  // Hard left correction
  if (s4) return 5;  // Hard right correction
  if (s1) return -1;  // Gentle left correction
  if (s3) return 1;  // Gentle right correction
  if (s2) return 0;  // Dead-centre, no correction needed

  return 99;  // No sensor active → line lost
}


// -------------------------------------------------------
// calculatePID — Compute PID output and update motors/servo
//
// Uses the signed error from calculateError() to adjust
// the left/right motor PWM values and the steering servo angle.
// Positive error → robot is right of line → slow left, speed up right.
// -------------------------------------------------------
void calculatePID(int error) {
  float P = error;             // Proportional term
  integral += error;           // Integral accumulator (Ki=0 so currently unused)
  float D = error - lastError; // Derivative term: rate of change of error

  // Combined PID correction
  float motorAdjustment = (Kp * P) + (Ki * integral) + (Kd * D);
  lastError = error;  // Save error for the next derivative calculation

  // Subtract adjustment from left, add to right so the robot steers
  // toward the line (positive adjustment → veer left → slow left motor)
  currentLeftSpeed = baseSpeed - motorAdjustment;
  currentRightSpeed = baseSpeed + motorAdjustment;

  // Keep speeds within 0-255 range
  currentLeftSpeed = constrain(currentLeftSpeed, 0, maxSpeed);
  currentRightSpeed = constrain(currentRightSpeed, 0, maxSpeed);

  driveMotors(currentLeftSpeed, currentRightSpeed);

  // Also save to last_speed[] for telemetry / debug
  last_speed[0] = currentLeftSpeed;
  last_speed[1] = currentRightSpeed;

  // Mirror the PID correction on the servo for visual feedback / fine steering.
  // servoCentre offset subtracted so positive error turns servo leftward.
  int targetAngle = (int)(servoCentre - (motorAdjustment*servoScale));
  targetAngle = constrain(targetAngle, servoMin, servoMax);
  myServo.write(targetAngle);
  
  delay(20);  // Short pause between PID cycles 
}

// -------------------------------------------------------
// obstacleDetected — Ultrasonic distance check
//
// Fires a single HC-SR04 pulse and returns true if an object
// is within `distance` cm. A 100 ms debounce prevents false
// triggers from a single noisy reading.
// Returns false immediately if called within 100 ms of last call.
// -------------------------------------------------------
bool obstacleDetected(int distance) {
  static unsigned long lastHit = 0;
  if (millis() - lastHit < 100) return false;  // Debounce guard
  lastHit = millis();

  // Send a 10 µs HIGH pulse to trigger the sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse width; timeout of 5000 µs ≈ 86 cm max range
  long duration = pulseIn(ECHO_PIN, HIGH, 5000);
  if (duration == 0) return false;  // No echo received within timeout

  // Convert to centimetres: distance = duration / 58
  float distanceCm = duration / 58;
  return distanceCm <= distance;
}

// -------------------------------------------------------
// driveMotors — Set motor speeds for forward motion
// motor1 (left)  — HIGH phase = forward
// motor2 (right) — LOW  phase = forward (wired in reverse)
// -------------------------------------------------------
void driveMotors(int left, int right) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, right);
}

// -------------------------------------------------------
// stopMotors — Cut power to both motors immediately
// -------------------------------------------------------
void stopMotors() {
  analogWrite(motor1PWM, 0);
  analogWrite(motor2PWM, 0);
}

// rest of the code is for dancing

void dancing(){
  delay(1000);
  turning();
  delay(500);
  twerking(10);
  delay(100);
  wiggle(15);
  delay(100);
}

void driveMotors_back(int left, int right) {
  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, right);
}

void twerking(int times) {
  for (int i = 0; i < times; i++) {
    driveMotors(250, 250); 
    delay(150);
    driveMotors_back(250, 250);
    delay(150);
  }
} 

void wiggle(int times){
  for (int i = 0; i < times; i++) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, 250);
  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, 250);
  
  delay(300);

  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, 250);
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, 250);
  delay(300);
  }
  stopMotors();
  delay(100);
}
