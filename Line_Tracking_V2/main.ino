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
    delay(100);}

  Serial.println("connected");
}

void loop() {
  int error = calculateError();

  if (!finished && obstacleDetected(15)) { // 15cm threshold
    printDistanceAndCheckTarget(10,1);
    performReroute();
    // After reroute, we return to top of loop to start PID line following immediately
    return; 
    }
    
  if(finished){
    stopMotors();
    while(true) delay(1000);
  }

  if (error == 100 && ((millis() - lasttalktoserver) > 500)){
    lasttalktoserver = millis();
    nodeEvent();
    }

  else if(error == 99){}

  else {
    calculatePID(error);
  }
  delay(20); 
}
