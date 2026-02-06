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
  bool obstacle = obstacleDetected();
  int error = calculateError();
  if (obstacle){
    Serial.println("obstacle");
    //delay(1000);
    turning();
    /*addEdge(path[pathIndex], path[pathIndex + 1], INF);
    findShortestPath(path[pathIndex], previousNodeID);*/
  }
  else{Serial.println("Clear");driveMotors(HIGH, HIGH);}
  delay(10);
  if(finished){
    stopMotors();
    delay(5000);
  }

  //int error = calculateError();
  
if (error == 100) {
    nodeEvent();
  }

  
  else if(error == 99){
    driveMotors(currentLeftSpeed, currentRightSpeed);
  }

  else {
    calculatePID(error);
  }
  delay(10); 
}
