#include "config.h"

int calculateError() {
  int threshold = 500;
  
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
  }


  bool s0 = AnalogValue[0] < threshold;
  bool s1 = AnalogValue[1] < threshold;
  bool s2 = AnalogValue[2] < threshold;
  bool s3 = AnalogValue[3] < threshold;
  bool s4 = AnalogValue[4] < threshold;

  if (s0 && s1 && s2 && s3 && s4) {
    return 100; 
  }

  if (s0) return -2;
  if (s4) return 2;
  if (s1) return -1;
  if (s3) return 1;
  if (s2) return 0;

  return 99; 
}

void calculatePID(int error) {
  float P = error;
  integral += error;
  float D = error - lastError;
  
  float motorAdjustment = (Kp * P) + (Ki * integral) + (Kd * D);
  lastError = error;

  currentLeftSpeed = baseSpeed - motorAdjustment;
  currentRightSpeed = baseSpeed + motorAdjustment;

  // Keep speeds within 0-255 range
  currentLeftSpeed = constrain(currentLeftSpeed, 0, maxSpeed);
  currentRightSpeed = constrain(currentRightSpeed, 0, maxSpeed);

  driveMotors(currentLeftSpeed, currentRightSpeed);

  last_speed[0] = currentLeftSpeed;
  last_speed[1] = currentRightSpeed;
}

void nodeEvent(){
  // 1. Identify where we are
  int arrivalNode;
  if (firstRun) {
    arrivalNode = 0; // Or 4, wherever you physically place the robot
  } else {
    arrivalNode = path[pathIndex];
    currentPosition = arrivalNode;
  }

  // 2. Check if we are at the end of the current route
  bool isFinalDestination = (pathIndex == pathLength - 1);

  if (firstRun || isFinalDestination) {

    if (!firstRun) {
      previousNodeID = path[pathIndex - 1]; 
    } 
    else {
      previousNodeID = 4; // Default for first run
    }

    // Talk to Server
    String response = notifyArrival(arrivalNode);
    
    if (response == "Finished" || response == "") {
      finished = true;
      return;
    }

    int targetNode = response.toInt();

    findShortestPath(arrivalNode, targetNode);
    
    firstRun = false;

    navigating();
  }
  else {
    // Do not talk to server. Just turn and keep driving.
    navigating();
  }
}

bool obstacleDetected() {
  static unsigned long lastHit = 0;
  if (millis() - lastHit < 400) return false;
  lastHit = millis();
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  //Serial.println("clear");
  long duration = pulseIn(ECHO_PIN, HIGH, 500);
  if (duration == 0) return false;

  float distanceCm = duration/29/2;
  return distanceCm <= OBSTACLE_DISTANCE_CM;
}

void driveMotors(int left, int right) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, right);
}


void stopMotors() {
  analogWrite(motor1PWM, 0);
  analogWrite(motor2PWM, 0);
}

/// rest of the code is for dancing

void dancing(){
  delay(1000);
  turning();
  delay(500);
  Nodding(10);
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

void Nodding(int times) {
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
