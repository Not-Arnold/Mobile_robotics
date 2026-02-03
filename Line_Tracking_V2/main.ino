void setup() {
  Serial.begin(115200);
  buildGraph();

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected");
    delay(100);}

  Serial.println("connected");
}

void loop() {
  if(finished){
    stopMotors();
    delay(5000);
  }

  int error = calculateError();
  
  if (error == 100) {
      // Only trigger navigation if we are NOT at the very end of our journey
      if (pathLength > 0 && pathIndex < pathLength - 1) {
        nagvigating();
      } else {
        // We've arrived at a waypoint (0, 1, 2, 3, or 4)
        // Ask server for the next destination
        if (millis() - lasttalktoserver > 500) {
          runClientLoop();
          pathIndex = 0; // Reset index for the new path
          lasttalktoserver = millis();
        }
      }
    }
  
  else if(error == 99){
    driveMotors(currentLeftSpeed, currentRightSpeed);
  }

  else {
    calculatePID(error);
  }
  delay(10); 
}
