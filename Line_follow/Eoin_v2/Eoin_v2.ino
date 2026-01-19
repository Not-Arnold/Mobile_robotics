int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;

int AnalogValue[5] = {0,0,0,0,0};
int AnalogPin[5] = {18,17, 16, 15, 7};
int last_speed[2] = {0,0};

int speed = 200;
float ratio = 0.93;

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
  forward(speed);
  }

void forward(int speed){
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, speed*ratio); 
  Serial.println("Forward"); 
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, speed); 
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

int left_speed(){
  float error = AnalogValue[2]/230;
  int left = 0;

  if (error <= 1){
    left = speed - speed*(9*(2700 - AnalogValue[0]) + (2700 - AnalogValue[1]))/(10*2700);
  }

  else {
    left = speed - speed*(7*(2700 - AnalogValue[0]) + 3*(2700 - AnalogValue[1]))/(10*2700);}

  return left;
}
//ismael was here
int right_speed(){
  float error = AnalogValue[2]/230;
  int right = 0;

  if (error <= 1){
    right = speed - speed*(9*(2700 - AnalogValue[4]) + (2700 - AnalogValue[3]))/(10*2700);
  }

  else {
    right = speed - speed*(7*(2700 - AnalogValue[4]) + 3*(2700 - AnalogValue[3]))/(10*2700);}

  return right;
}

void loop(){
  line_detection();
  

  if (AnalogValue[4] < 500 and AnalogValue[0] < 500){
    forward(speed);
    //nodeReached();
  }

  else if (AnalogValue[4] > 2000 and AnalogValue[3] > 2000 and AnalogValue[2] > 2000 and AnalogValue[1] > 2000 and AnalogValue[0] > 2000){
    int L = last_speed[0];
    int R = last_speed[1];

    if (L > R){
    digitalWrite(motor2Phase, LOW);
    analogWrite(motor2PWM, 255);

    digitalWrite(motor1Phase, HIGH);
    analogWrite(motor1PWM, 50);}

    else {
    digitalWrite(motor2Phase, LOW);
    analogWrite(motor2PWM, 50);

    digitalWrite(motor1Phase, HIGH);
    analogWrite(motor1PWM, 255);}

    }
  
  
  else{
    int L = left_speed();
    int R = right_speed();

    digitalWrite(motor2Phase, LOW);
    analogWrite(motor2PWM, L);

    digitalWrite(motor1Phase, HIGH);
    analogWrite(motor1PWM, R);

    last_speed[0] = L;
    last_speed[1] = R;}

    delay(50);

}