#define BUZZER_PIN 4   // use a safe GPIO

int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;

int AnalogPin[5] = {18, 17, 16, 15, 7};
int AnalogValue[5] = {0, 0, 0, 0, 0};
int last_speed[2] = {0,0};


float Kp = 120; 
float Ki = 0.05;
float Kd = 150;

int lastError = 0;
float integral = 0;
int baseSpeed = 255; 
int maxSpeed = 255;

//NOTES FOR BUZZER
const int c4 = 262;
const int d4 = 294;
const int e4 = 330;
const int f4 = 349;
const int g4 = 392;
const int a4 = 440;
const int b4 = 494;
const int c5 = 523;
const int rest = 0;

//FUNCTION TO PLAY NOTE
void playNote(int freq, int dur) {
  if (freq == rest) {
    delay(dur);
    return;
  }

  long period = 1000000L / freq;   // microseconds per wave
  long halfPeriod = period / 2;
  long cycles = (dur * 1000L) / period;

  for (long i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(halfPeriod);
  }

  delay(20); // small gap
}

//Drums of Liberation Song for start up
void playGear5Chorus() {

  playNote(a4, 300); 
  playNote(a4, 300);
  playNote(g4, 550);

  playNote(rest, 100);

  playNote(g4, 225);
  playNote(e4, 225); 
  playNote(c4, 250);
  playNote(a4, 550);

  playNote(rest, 200);

  playNote(a4, 300); 
  playNote(e4, 300);
  playNote(d4, 650);

  playNote(rest, 100);

  playNote(g4, 300); 
  playNote(a4, 550);

  playNote(rest, 100);
}

//Accomplishment sound when node is reached - Final Fantasy
void nodeReached(){
  playNote(b4, 150); 
  playNote(b4, 150);
  playNote(b4, 150);
  playNote(b4, 175);

  playNote(rest, 125);

  playNote(c4, 150);
  playNote(f4, 150); 
  
  playNote(rest, 125);

  playNote(b4, 125);
  playNote(f4, 125);
  playNote(b4, 125);
}



void setup() {
  Serial.begin(115200);

  // Buzzer startup
  pinMode(BUZZER_PIN, OUTPUT);
  //playGear5Chorus();

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);
  }
}

unsigned long lastPrintTime = 0;
bool number = 0;
void song(){
  if (!number){
  playGear5Chorus();
  number=1;}
}


void loop() {
  song();
  
  int error = calculateError();
  if (error == 100) {
    driveMotors(last_speed[0], last_speed[1]);
  } else {
    calculatePID(error);
  }

  if (millis() - lastPrintTime > 600) {
    for (int i = 0; i < 5; i++) {
      Serial.print(AnalogValue[i]);
      Serial.print("\t");
    }
    Serial.print("Error: ");
    Serial.println(error);
    
    lastPrintTime = millis(); // Reset the timer
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
    return 0; 
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

  int leftSpeed = baseSpeed - motorAdjustment;
  int rightSpeed = baseSpeed + motorAdjustment;

  // Keep speeds within 0-255 range
  leftSpeed = constrain(leftSpeed, 0, maxSpeed);
  rightSpeed = constrain(rightSpeed, 0, maxSpeed);

  driveMotors(leftSpeed, rightSpeed);

  last_speed[0] = leftSpeed;
  last_speed[1] = rightSpeed;
}

void driveMotors(int left, int right) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, right);
}

void stopMotors() {
  analogWrite(motor1PWM, 0);
  analogWrite(motor2PWM, 0);
}
