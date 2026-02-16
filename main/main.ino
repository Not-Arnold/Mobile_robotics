void setup() {
  Serial.begin(115200);
  buildGraph();

  myServo.attach(SERVO_PIN);

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  myServo.write(servoCentre);
  
  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected");
    delay(100);
  }

  Serial.println("connected");
  resetServerMap();
  // Start the background task on Core 0
  setupMultiCore();
}

void loop() {
  if(finished){
    stateStr = "FINISHED";
    stopMotors();
    while(true) delay(1000);
    return;
  }

  if (millis() - lastTelemetryTime > 1000) { // Increase to 500ms to give the CPU more breathing room
    lastTelemetryTime = millis();
    //sendTelemetry();
  }

  int error = calculateError();

  if (!finished && obstacleDetected(15)) { // 15cm threshold
    obstacleStartNode = currentPosition;
    obstacleEndNode = nextNode;
    stateStr = "OBSTACLE DETECTED";

    performReroute();

    // After reroute, we return to top of loop to start PID line following immediately
    return; 
  }

  if (error == 100 && ((millis() - lasttalktoserver) > 500)){
    lasttalktoserver = millis();
    stateStr = "AT NODE";
    nodeEvent();
    //sendTelemetry();
  } else if(error == 99){
    stateStr = "LOST LINE";
  } else {
    stateStr = "LINE FOLLOWING";
    calculatePID(error);
  }
  delay(20); 
}
