void setup() {
  Serial.begin(115200);

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
  int error = calculateError();

  if (error == 100) {
      stopMotors();
      
      // Save the node we are leaving before updating
      int prevNode = currentPosition; 

      // Get the next node from the server
      runClientLoop(); 
      int nextNode = currentPosition; 

      // MANUALLY DETERMINE THE JUNCTION (6 or 7)
      int junction = -1;
      
      // If traveling between bottom loop (0,2) and middle (1), use Junction 6
      if ((prevNode == 0 || prevNode == 2 || prevNode == 1) && 
          (nextNode == 0 || nextNode == 2 || nextNode == 1)) {
          junction = 6;
      } 
      // If traveling between top loop (3,4) and middle (1), use Junction 7
      else if ((prevNode == 3 || prevNode == 4 || prevNode == 1) && 
              (nextNode == 3 || nextNode == 4 || nextNode == 1)) {
          junction = 7;
      }

      // Now call getTurn with the 'injected' junction node
      String direction = "STRAIGHT";
      if (junction != -1) {
          direction = getTurn(prevNode, junction, nextNode);
      }

      // EXECUTE ACTION
      if (direction == "LEFT") {
          turningL();
      } else if (direction == "RIGHT") {
          turningR();
      } else {
          // DRIVE STRAIGHT: Move forward slightly to clear the line
          driveMotors(baseSpeed, baseSpeed);
          delay(400); 
      }
  }
  
  else {
    calculatePID(error);
  }

if (millis() - lastPrintTime > 600) {
  Serial.print("Sensors: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(AnalogValue[i]);
    Serial.print("\t");
  }

  Serial.print("| Err: ");
  Serial.print(error);
  Serial.print(" | L: ");
  Serial.print(currentLeftSpeed);
  Serial.print(" | R: ");
  Serial.println(currentRightSpeed);

  lastPrintTime = millis();
}

  delay(10); 
}
