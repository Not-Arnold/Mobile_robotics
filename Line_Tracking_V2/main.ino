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
  int error = calculateError();

  if(finished){
    stopMotors();
    while(true) delay(1000);
  }

  if (error == 100 && ((millis() - lasttalktoserver) > 300)){
    lasttalktoserver = millis();
    nodeEvent();
    }
  

  else if(error == 99){
    driveMotors(currentLeftSpeed, currentRightSpeed);
  }

  else {
    calculatePID(error);
  }
  //delay(10); 
}
