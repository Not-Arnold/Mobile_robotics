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

    if(millis() - lasttalktoserver > 500){
      stopMotors();
      
      // Save the node we are leaving before updating
      int prevNode = currentPosition; 

      // Get the next node from the server
      if (nextturn == 0) {runClientLoop(); }
   
      int nextNode = currentPosition; 

      String direction = getTurn(prevNode, nextNode);

      if (nextturn == 1){
        if (direction == "LEFT") {
            turningL();
            nextturn = 0;}

        else if (direction == "RIGHT") {
            turningR();
            nextturn = 0;}}

        else if (direction == "STRAIGHT") {
          driveMotors(baseSpeed, baseSpeed);
          delay(300);
          nextturn = 0;}

       else {
          // DRIVE STRAIGHT: Move forward slightly to clear the line
          driveMotors(baseSpeed, baseSpeed);
          delay(300);
      }
      
      lasttalktoserver = millis();
  }}
  
  else if(error == 99){}

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
