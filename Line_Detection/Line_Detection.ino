//Motor control
const int motor1PWM = 37;
const int motor1Phase = 38;
const int motor2PWM = 39;
const int motor2Phase = 20;

const int MAXPWH = 200;
const float ratio = 0.93;

int AnalogValue[5] = {0,0,0,0,0};
const int AnalogPin[5] = {18, 17, 16, 15, 7}; // keep 8 free for tone O/P music
const int weights[5] = {-2, -1, 0, 1, 2};
bool s[5];

int baseSpeed = 140;      // cruise speed (tune)
float Kp = 18.0;          // proportional gain (tune) How sharp you turn 55.0
float Kd = 6.0;          // derivative gain (tune) How fast you turn 25.0
float lastError = 0;

int lastSeenDir = 1;      // +1 = last line on right, -1 = last line on left

const int AVG_N = 15;

float errHist[AVG_N] = {0};
float errSum = 0;
int errIdx = 0;
int errCount = 0;

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
}
*/
float addErrorAndGetAvg(float e) {
  // remove oldest
  errSum -= errHist[errIdx];
  // add newest
  errHist[errIdx] = e;
  errSum += e;

  errIdx = (errIdx + 1) % AVG_N;
  if (errCount < AVG_N) errCount++;

  return errSum / errCount;
}


int MAXspeed(int speed)
{
  if (speed < 0){return 0;}
  //else if (speed > MAXPWH){return MAXPWH;}
  return speed;
}

void forward(int L_speed = MAXPWH, int R_speed = MAXPWH)
{
  L_speed = MAXspeed(L_speed);
  R_speed = MAXspeed(R_speed);

  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, L_speed*ratio); 
  //Serial.println("Forward"); 
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, R_speed); 
 
}

void backward(int L_speed = MAXPWH, int R_speed = MAXPWH)
{
  L_speed = MAXspeed(L_speed);
  R_speed = MAXspeed(R_speed);

  digitalWrite(motor1Phase, LOW);
  analogWrite(motor1PWM, L_speed*ratio);
  //Serial.println("Backward");
  digitalWrite(motor2Phase, HIGH);
  analogWrite(motor2PWM, R_speed);
 
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);
}

void loop() {
  // 1) Read sensors + threshold to boolean
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);

    // keep your threshold rule, flip if needed
    s[i] = (AnalogValue[i] < 500);
  }
  Serial.println();
  //delay(10); // faster update helps control

  // 2) Compute weighted position of line
  int weightedSum = 0;
  int activeCount = 0;

  for (int i = 0; i < 5; i++) {
    if (s[i]) {
      weightedSum += weights[i];  // weights: -2,-1,0,1,2
      activeCount++;
    }
  }

  // 3) Handle "line lost" (no sensors active)
  if (activeCount == 0) {
    // Search in the last seen direction
    if (lastSeenDir < 0) {
      // turn left slowly to find line
      forward(0, 120);
      Serial.println("Searching left");
    } else {
      // turn right slowly to find line
      forward(120, 0);
      Serial.println("Searching right");
    }
    return;
  }
  else if (activeCount == 5){Serial.println("NODE");} //delay(100);}

  float pos = (float)weightedSum / (float)activeCount;  // -2 .. +2
  float error = pos;  // target is 0 (middle sensor on line)

  // Moving average smoothing (last 20 errors)
  float avgError = addErrorAndGetAvg(error);

  // remember which side line was on (for recovery)
  if (avgError < -0.2) lastSeenDir = 1;
  else if (avgError > 0.2) lastSeenDir = -1;

  // 4) PD control (P-only also works if you set Kd=0)
  float derivative = avgError - lastError;
  lastError = avgError;

  float correction = (Kp * avgError) + (Kd * derivative);

  // 5) Convert correction to motor speeds
  int L = (int)(baseSpeed + correction);
  int R = (int)(baseSpeed - correction);

  // --- If it turns the wrong way, flip the sign by swapping these:
  // int L = (int)(baseSpeed + correction);
  // int R = (int)(baseSpeed - correction);

  forward(R, L);

  // Debug
  Serial.print("pos="); Serial.print(pos);
  Serial.print(" err="); Serial.print(error);
  Serial.print(" corr="); Serial.print(correction);
  Serial.print(" L="); Serial.print(L);
  Serial.print(" R="); Serial.println(R);
}
