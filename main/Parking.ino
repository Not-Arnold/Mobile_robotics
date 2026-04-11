// ============================================================
// Parking.ino — Final Parking Approach
//
// Handles the robot's final movement into the parking bay.
// Uses the line sensors to stay aligned while following the line,
// then switches to ultrasonic-distance-based stopping once the
// line is lost / parking branch is entered.
// ============================================================


// -------------------------------------------------------
// readDistanceCm — Measure distance using the ultrasonic sensor
//
// Procedure:
//   1. Send a short trigger pulse on TRIG_PIN
//   2. Measure the echo pulse width on ECHO_PIN
//   3. Convert the pulse duration to centimetres
//
// Returns:
//   - distance in cm
//   - -1.0 if no echo is received within the timeout
// -------------------------------------------------------
float readDistanceCm(){
  // Trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse width (timeout in microseconds)
  // 5000us ~ up to ~5m (safe), adjust shorter if you want less blocking
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 5000);

  if (duration == 0) return -1.0;     // no echo / out of range

  // Convert us to cm: distance(cm) = duration / 58.0
  return duration / 58.0;
}

// -------------------------------------------------------
// printDistanceAndCheckTarget — Debug-print current distance
// and test whether the target parking distance has been reached
//
// Parameters:
//   targetCm    = desired stopping distance
//   toleranceCm = acceptable error band around target
//
// Returns:
//   true  if current distance is within tolerance
//   false otherwise
//
// Note:
//   Printing is rate-limited to once every 100 ms so Serial
//   output does not flood the loop.
// -------------------------------------------------------
bool printDistanceAndCheckTarget(float targetCm, float toleranceCm = 1.0){
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 100) return false; // print at most every 100ms
  lastPrint = millis();

  float d = readDistanceCm();

  if (d < 0) {
    Serial.println("Distance: --- (no echo)");
    return false;
  }

  // Print current measurement and target window
  Serial.print("Distance: ");
  Serial.print(d, 1);
  Serial.print(" cm  | Target: ");
  Serial.print(targetCm, 1);
  Serial.print(" ± ");
  Serial.print(toleranceCm, 1);
  Serial.println(" cm");

  // Check whether parking target has been reached
  if (fabs(d - targetCm) <= toleranceCm) {
    Serial.println(">>> TARGET REACHED <<<");
    return true;
  }

  return false;
}

// -------------------------------------------------------
// driveStraightToParking — Perform the final parking manoeuvre
//
// Behaviour:
//   1. Reduce base speed for safer parking
//   2. Continue line-following while the line is still visible
//   3. Once the line is lost, use ultrasonic distance to drive
//      straight toward the parking end point
//   4. Slow down when close to the wall / stop point
//   5. Stop, notify arrival at node 5, and mark the run finished
// -------------------------------------------------------
void driveStraightToParking() {
  // Reduce general speed limits for parking mode
  baseSpeed = 100; 
  maxSpeed = 150;
  
  Serial.println("Parking Mode Initiated...");

  // Tracks whether we are still using the line for guidance
  bool on_line = true;

  // Parking motion parameters
  int driveSpeed = 150;     // Moderate speed to start
  int creepSpeed = 30;     // Slow speed for precision
  float stopDist = 5;     // Stop at 5 cm (sensors can be jittery <2cm)
  float slowDist = 10.0;    // Start slowing down at 10cm

  while (true) {
    // Get line-following error from the sensor array
    int error = calculateError();

    // If line is detected normally, keep using PID steering
    if ((error != 99) && (error != 100) && on_line){calculatePID(error);}

    // Special case: centered / neutral reading
    else if ((error == 100) && on_line){calculatePID(0);}

    // If line is lost, switch into distance-based parking mode
    if ((error == 99) || (on_line == false)){
      on_line = false;

      // Measure current distance to obstacle / end wall
      float dist = readDistanceCm();

      // Print debug info and target check
      printDistanceAndCheckTarget(5.0, 1.0);
      
      // Safety fallback: treat invalid readings as "far away"
      if (dist <= 0) dist = 999.0; 

      // Stop once close enough to the parking end point
      if (dist <= stopDist) {
        notifyArrival(5);   // Tell server that parking node has been reached
        finished = true;
        Serial.println("Parked.");

        stopMotors();

        // Hold position permanently
        while(true) {stopMotors();}
        break;
      } else if (dist < slowDist) {
        // Approaching wall - Slow down
        driveMotors(creepSpeed*0.9, creepSpeed);
      } else {
        // Driving to wall
        driveMotors(driveSpeed*0.9, driveSpeed);
      }
    }
    delay(20); // Small stability delay
  }
}
