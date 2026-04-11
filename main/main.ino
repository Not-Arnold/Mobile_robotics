// ============================================================
// main.ino — Entry Point: setup() and loop()
// Initialises all hardware, connects to WiFi, then runs the
// main control cycle: obstacle detection → node events → PID.
// ============================================================

void setup() {
  Serial.begin(115200);  // Open serial port for debug output
  buildGraph();  // Construct the adjacency list (Algo.ino)

  // --- SERVO INIT ---
  myServo.attach(SERVO_PIN);
  
  // --- MOTOR PINS ---
  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  // --- ULTRASONIC SENSOR PINS ---
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);  // Ensure trigger starts LOW

  myServo.write(servoCentre);  // Centre servo before driving

  // --- LINE SENSOR PINS ---
  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);
  }

   // --- WIFI CONNECTION ---
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected");
    delay(100);
  }

  Serial.println("connected");
  resetServerMap();   // Clear any leftover obstacle data on the dashboard
  setupMultiCore();   // Launch telemetry task on Core 0 (Wifi.ino)
}

void loop() {
  // --- TERMINATION CHECK ---
  // Once the robot has finished its delivery, stop permanently.
  if(finished){
    stateStr = "FINISHED";
    stopMotors();
    while(true) delay(1000);
    return;
  }

  // --- PERIODIC TELEMETRY (optional, currently disabled) ---
  // sendTelemetry() is handled by the Core 0 task instead.
  if (millis() - lastTelemetryTime > 1000) { // Increase to   1000ms to give the CPU more breathing room
    lastTelemetryTime = millis();
    //sendTelemetry();
  }

  // Read all 5 IR sensors and calculate the line-position error
  int error = calculateError();

  // --- OBSTACLE DETECTION ---
  // If an object is within 15 cm, trigger an emergency reroute.
  if (!finished && obstacleDetected(15)) { // 15cm threshold
    obstacleStartNode = currentPosition;  // Record where we were
    obstacleEndNode = nextNode;         // Record where we were heading
    stateStr = "OBSTACLE DETECTED";

    performReroute();  // Block edge, U-turn, replan path (Algo.ino)

    // After reroute, we return to top of loop to start PID line following immediately
    return; 
  }

  // --- NODE / LINE STATE MACHINE ---
  if (error == 100 && ((millis() - lasttalktoserver) > 500)){
    // All sensors dark → robot is sitting on a junction node.
    // Rate-limit server calls to once per 500 ms.
    lasttalktoserver = millis();
    stateStr = "AT NODE";
    nodeEvent();         // Decide next action / talk to server (Algo.ino)

  } else if(error == 99){
    // No sensors detect the line → robot has lost the track.
    stateStr = "LOST LINE";
    
  } else {
    // Normal line-following: apply PID correction to motors & servo.
    stateStr = "LINE FOLLOWING";
    calculatePID(error);
  }
  delay(20);   // Small yield to avoid watchdog resets
}
