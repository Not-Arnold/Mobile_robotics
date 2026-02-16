// --- NEW PARKING FUNCTIONS ---

// Helper to get accurate distance
float readDistanceCm()
{
  // Trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse width (timeout in microseconds)
  // 30000us ~ up to ~5m (safe), adjust shorter if you want less blocking
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 5000);

  if (duration == 0) return -1.0;     // no echo / out of range

  // Convert us to cm: distance(cm) = duration / 58.0
  return duration / 58.0;
}


bool printDistanceAndCheckTarget(float targetCm, float toleranceCm = 1.0)
{
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 100) return false; // print at most every 100ms
  lastPrint = millis();

  float d = readDistanceCm();

  if (d < 0) {
    Serial.println("Distance: --- (no echo)");
    return false;
  }

  Serial.print("Distance: ");
  Serial.print(d, 1);
  Serial.print(" cm  | Target: ");
  Serial.print(targetCm, 1);
  Serial.print(" ± ");
  Serial.print(toleranceCm, 1);
  Serial.println(" cm");

  if (fabs(d - targetCm) <= toleranceCm) {
    Serial.println(">>> TARGET REACHED <<<");
    return true;
  }

  return false;
}


void driveStraightToParking() {
  baseSpeed = 100; 
  maxSpeed = 150;
  
  Serial.println("Parking Mode Initiated...");
  bool on_line = true;

  int driveSpeed = 150;     // Moderate speed to start
  int creepSpeed = 30;     // Slow speed for precision
  float stopDist = 5;     // Stop at 2-4cm (sensors can be jittery <2cm)
  float slowDist = 10.0;    // Start slowing down at 15cm

  while (true) {
    int error = calculateError();

    if ((error != 99) && (error != 100) && on_line){
    calculatePID(error);}

    else if ((error == 100) && on_line){
      calculatePID(0);}

    if ((error == 99) || (on_line == false)){
        on_line = false;
        float dist = readDistanceCm();
        printDistanceAndCheckTarget(5.0, 1.0);
        
        // Safety break if sensor is acting up (reading 0 often means error)
        if (dist <= 0) dist = 999.0; 

        
        if (dist <= stopDist) {
          notifyArrival(5);
          finished = true;
          Serial.println("Parked.");

          stopMotors();
          while(true) {
          stopMotors();}
          break;
        }

        else if (dist < slowDist) {
          // Approaching wall - Slow down
          driveMotors(creepSpeed*0.9, creepSpeed);
        } 


        else {
          // Driving to wall
          driveMotors(driveSpeed*0.9, driveSpeed);
        }}
        delay(20); // Small stability delay
      }
}