#define BUZZER_PIN 4   // use a safe GPIO

int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;

int AnalogPin[5] = {18, 17, 16, 15, 7};
int AnalogValue[5] = {0, 0, 0, 0, 0};
int last_speed[2] = {0,0};

int currentLeftSpeed = 0;   // Add these here
int currentRightSpeed = 0;  // so loop() can see them

float Kp = 100; 
float Ki = 0.0;
float Kd = 80;

int lastError = 0;
float integral = 0;
int baseSpeed = 255; 
int maxSpeed = 255;


void setup() {
  Serial.begin(115200);

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);
  }
}

unsigned long lastPrintTime = 0;


void loop() {
  int error = calculateError();
  if (error == 100) {
    //driveMotors(last_speed[0], last_speed[1]);
    //stopMotors();

  } else {
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

int calculateError() {
  int threshold = 1000;
  
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
  }


  bool s0 = AnalogValue[0] < threshold;
  bool s1 = AnalogValue[1] < threshold;
  bool s2 = AnalogValue[2] < threshold;
  bool s3 = AnalogValue[3] < threshold;
  bool s4 = AnalogValue[4] < threshold;

  if (s0 && s1 && s2 && s3 && s4) {
    stopMotors();
    delay(100);
  }

  if (s0) return -2;
  if (s4) return 2;
  if (s1) return -1;
  if (s3) return 1;
  if (s2) return 0;

  return 100; 
}

void calculatePID(int error) {
  float P = error;
  integral += error;
  float D = error - lastError;
  
  float motorAdjustment = (Kp * P) + (Ki * integral) + (Kd * D);
  lastError = error;

  currentLeftSpeed = baseSpeed - motorAdjustment;
  currentRightSpeed = baseSpeed + motorAdjustment;

  // Keep speeds within 0-255 range
  currentLeftSpeed = constrain(currentLeftSpeed, 0, maxSpeed);
  currentRightSpeed = constrain(currentRightSpeed, 0, maxSpeed);

  driveMotors(currentLeftSpeed, currentRightSpeed);

  last_speed[0] = currentLeftSpeed;
  last_speed[1] = currentRightSpeed;
}

void driveMotors(int left, int right) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, right);
}

void turning(){
int turnSpeed = 200; // A manageable speed for rotating
  int threshold = 1000;

  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, turnSpeed);

  delay(300); 


  while (analogRead(AnalogPin[2]) > threshold) {
  }

  stopMotors();
  delay(200);
}

void stopMotors() {
  analogWrite(motor1PWM, 0);
  analogWrite(motor2PWM, 0);
}

/// rest of the code is for dancing

void dancing(){
  delay(1000);
  turning();
  delay(500);
  twerking(10);
  delay(100);
  wiggle(15);
  delay(100);
}


void driveMotors_back(int left, int right) {
  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, right);
}


void twerking(int times) {
  for (int i = 0; i < times; i++) {
    driveMotors(250, 250); 
    delay(150);
    driveMotors_back(250, 250);
    delay(150);
  }}
  

void wiggle(int times){
  for (int i = 0; i < times; i++) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, 250);
  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, 250);
  
  delay(300);

  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, 250);
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, 250);
  delay(300);
  }
  stopMotors();
  delay(100);
}
