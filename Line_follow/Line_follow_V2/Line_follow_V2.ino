
#define BUZZER_PIN 4   // use a safe GPIO and add 4.70 KΩ resistor in series to buzzer

//Motor control
const int motor1PWM = 37;
const int motor1Phase = 38;
const int motor2PWM = 39;
const int motor2Phase = 20;

const int MAXPWH = 200;
const float ratio = 0.93;
int turn = 40;

int AnalogValue[5] = {0,0,0,0,0};
const int AnalogPin[5] = {7, 15, 16, 17, 18}; // keep 8 free for tone O/P music
const int weights[5] = {-2, -1, 0, 1, 2};
bool s[5];

int baseSpeed = 140;      // cruise speed (tune)
float Kp = 55.0;          // proportional gain (tune)
float Kd = 25.0;          // derivative gain (tune)
float lastError = 0;

int lastSeenDir = 1;      // +1 = last line on right, -1 = last line on left

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

int MAXspeed(int speed)
{
  if (speed < 0){return 0;}
  else if (speed > MAXPWH){return MAXPWH;}
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

  // buzzer setup
  pinMode(BUZZER_PIN, OUTPUT);
  playGear5Chorus();

  // Motor setup
  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  for (int i = 0; i<5; i++){

    AnalogValue[i]=analogRead(AnalogPin[i]);
    Serial.print(AnalogValue[i]); // This prints the actual analog sensor reading
    Serial.print("\t"); //tab over on screen

    if (AnalogValue[i]<500){s[i]=1;}
    else {s[i]=0;}
  }

  Serial.println(""); //carriage return
  delay(100); // display new set of readings every 600mS

  int sum = 0;
  for (int i = 0; i < 5; i++){
    if (s[i] == 1){
      sum += weights[i];
    }
  }

  if (sum == 0){
    forward();
    Serial.println("Straight ahead");
    }
  else{
    if (sum<0){
      forward(MAXPWH - sum*40, MAXPWH + sum*turn);
      Serial.println("Left turn");
    }
    else{
      forward(MAXPWH - sum*40, MAXPWH + sum*turn);
      Serial.println("Right turn");
    }
  }
}
