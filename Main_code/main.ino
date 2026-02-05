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
    stopMotors(); 
    Serial.println("Node Detected.");

    // 1. Identify where we are
    int arrivalNode;
    if (firstRun) {
      arrivalNode = 0; // Or 4, wherever you physically place the robot
    } else {
      arrivalNode = path[pathIndex];
      currentPosition = arrivalNode;
    }

    Serial.print("Arrived at ID: ");
    Serial.println(arrivalNode);

    // 2. Check if we are at the end of the current route
    bool isFinalDestination = (pathIndex == pathLength - 1);

    if (firstRun || isFinalDestination) {

      if (!firstRun) {
            previousNodeID = path[pathIndex - 1]; 
        } else {
            previousNodeID = 4; // Default for first run
        }

        // Talk to Server
        String response = notifyArrival(arrivalNode);
        
        if (response == "Finished" || response == "") {
          finished = true;
          return;
        }

        int targetNode = response.toInt();
        Serial.print("Server says go to: ");
        Serial.println(targetNode);

        findShortestPath(arrivalNode, targetNode);
        
        firstRun = false;

        navigating(); 
        
    } 
    else {
        // --- WE ARE AT AN INTERMEDIATE NODE (6 or 7) ---
        // Do not talk to server. Just turn and keep driving.
        Serial.println("Intermediate Node - Keep Going");
        navigating();
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
