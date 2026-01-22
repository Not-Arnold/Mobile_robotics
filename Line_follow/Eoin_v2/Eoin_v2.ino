int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;

int AnalogValue[5] = {0,0,0,0,0};
int AnalogPin[5] = {18,17, 16, 15, 7};
int last_speed[2] = {0,0};

int speed = 200;
float ratio = 0.93;

int max_Dark = 2200;

int floorVal[3] = {0,0,0};   // L,C,R floor readings
int lineVal[3]  = {2700,2700,2700}; // L,C,R line readings (max)

/*#define BUZZER_PIN 4 // use a safe GPIO

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
}*/

void setup() {
  Serial.begin(9600);
  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);
  //playGear5Chorus();
  floorVal[0] = analogRead(AnalogPin[1]);
  floorVal[1] = analogRead(AnalogPin[2]);
  floorVal[2] = analogRead(AnalogPin[3]);
  driveForwardLR(speed, speed);
  }

void driveForwardLR(int L_speed, int R_speed){
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, L_speed*ratio); 
  Serial.println("Forward"); 
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, R_speed); 
  Serial.println("Forward"); 
}

void backword(int speed){
  digitalWrite(motor1Phase, LOW);
  analogWrite(motor1PWM, speed*ratio);
  Serial.println("Backward");
  digitalWrite(motor2Phase, HIGH);
  analogWrite(motor2PWM, speed);
  Serial.println("Backward");
}

void line_detection(){
  for (int i=0; i<5; i++){
    AnalogValue[i] = analogRead(AnalogPin[i]);
    Serial.print(AnalogValue[i]);
    Serial.print("\t");

      if (i==4){
        Serial.println("");  
}}}

float norm(int raw, int floorR){
  float v = (raw - floorR) / floorVal[0];  // 2000 is scale; adjust if needed
  if (v < 0) v = 0;
  if (v > 1) v = 1;
  return v;
}

int left_speed(){
  float error = AnalogValue[2]/230;
  int left = 0;

  if (error <= 1){
    left = speed - speed*(9*(max_Dark - AnalogValue[0]) + (max_Dark - AnalogValue[1]))/(10*max_Dark);
  }

  else {
    left = speed - speed*(7*(max_Dark - AnalogValue[0]) + 3*(max_Dark - AnalogValue[1]))/(10*max_Dark);}

  return left;
}

int left_speed_3(){
  float lineL = (AnalogValue[1]);
  float lineC = (AnalogValue[2]);
  float lineR = (AnalogValue[3]);

  float L = lineL / max_Dark;
  float C = lineC / max_Dark;
  float R = lineR / max_Dark;

  float sum = L + C + R;
  if (sum < 0.001) sum = 0.001;

  float err = (-1.8 * L + 0.0 * C + 1.8 * R) / sum;

  float e = err;
  float boost = (fabs(e) > 0.35) ? 1.8 : 1.0;
  float correction = 120.0 * e * boost;

  int left = (int)(speed - correction);

  if (left < 20) left = 20;
  if (left > 255) left = 255;

  return left;
}

int right_speed_3(){
  // Convert readings into "how much line is under sensor"
  // If your line is DARK and gives HIGH values, keep as-is.
  // If line gives LOW values, flip like: lineL = max_Dark - AnalogValue[1]; etc.
  float lineL = (AnalogValue[1]);   // left
  float lineC = (AnalogValue[2]);   // center
  float lineR = (AnalogValue[3]);   // right

  // Normalise 0..1
  float L = lineL / max_Dark;
  float C = lineC / max_Dark;
  float R = lineR / max_Dark;

  // Position error using 3 sensors:
  // negative -> line is left, positive -> line is right
  float sum = L + C + R;
  if (sum < 0.001) sum = 0.001;

  float err = (-1.8 * L + 0.0 * C + 1.8 * R) / sum;  // stronger outer weight

  // Non-linear boost for sharp turns
  float e = err;
  float boost = (fabs(e) > 0.35) ? 1.8 : 1.0;        // turn harder when error is big
  float correction = 120.0 * e * boost;              // 120 sets strength

  int right = (int)(speed + correction);

  // clamp
  if (right < 20) right = 20;
  if (right > 255) right = 255;

  return right;
}

//ismael was here
int right_speed(){
  float error = AnalogValue[2]/230;
  int right = 0;

  if (error <= 1){
    right = speed - speed*(9*(max_Dark - AnalogValue[4]) + (max_Dark - AnalogValue[3]))/(10*max_Dark);
  }

  else {
    right = speed - speed*(7*(max_Dark - AnalogValue[4]) + 3*(max_Dark - AnalogValue[3]))/(10*max_Dark);}

  return right;
}

void loop(){
  line_detection();
  

  float L = norm(AnalogValue[1], floorVal[0]);
float C = norm(AnalogValue[2], floorVal[1]);
float R = norm(AnalogValue[3], floorVal[2]);

float sum = L + C + R;
if (sum < 0.001) sum = 0.001;

// error: -1..+1 (ish)
float error = (R - L) / sum;

// deadband for straight
if (fabs(error) < 0.08) error = 0;

// adaptive gain: small near center, big when off-center
float gain = 80;                 // normal
if (fabs(error) > 0.35) gain = 160;   // sharper turns
if (fabs(error) > 0.60) gain = 220;   // very sharp

int correction = (int)(gain * error);

int leftPWM  = speed - correction;
int rightPWM = speed + correction;

// clamp (avoid stalling)
leftPWM  = constrain(leftPWM,  40, 255);
rightPWM = constrain(rightPWM, 40, 255);

// drive (match your motor mapping)
driveForwardLR(leftPWM, rightPWM);


}