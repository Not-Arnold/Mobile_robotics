int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;
// the setup routine runs once when you press reset:
void setup() {
Serial.begin(9600);
pinMode(motor1PWM, OUTPUT);
pinMode(motor1Phase, OUTPUT);

pinMode(motor2PWM, OUTPUT);
pinMode(motor2Phase, OUTPUT);
}
// the loop routine runs over and over again continuously:
void loop() {
int speed = 0;
delay(200);
while (speed<255){
  Serial.println(speed);
  digitalWrite(motor1Phase, HIGH); //forward
  analogWrite(motor1PWM, speed); // set speed of motor
  digitalWrite(motor2Phase, LOW); //forward
  analogWrite(motor2PWM, speed); // set speed of motor
  delay(100);
  speed+=10;
}

speed = 255;
for (int i = 1; i<26; i++){
  Serial.println(speed);
  digitalWrite(motor1Phase, HIGH); //forward
  analogWrite(motor1PWM, speed); // set speed of motor
  digitalWrite(motor2Phase, LOW); //forward
  analogWrite(motor2PWM, speed); // set speed of motor
  speed = speed -10;
  delay(100);
}

Serial.println("Forward"); // Display motor direction
delay(2000); //2 seconds

speed = 0;
while (speed<255){
  Serial.println(speed);
  digitalWrite(motor1Phase, LOW); //forward
  analogWrite(motor1PWM, speed); // set speed of motor
  digitalWrite(motor2Phase, HIGH); //forward
  analogWrite(motor2PWM, speed); // set speed of motor
  delay(100);
  speed+=10;
}
speed = 255;

for (int i = 1; i<26; i++){
  Serial.println(speed);
  digitalWrite(motor1Phase, LOW); //forward
  analogWrite(motor1PWM, speed); // set speed of motor
  digitalWrite(motor2Phase, HIGH); //forward
  analogWrite(motor2PWM, speed); // set speed of motor
  speed = speed -10;
  delay(100);
}

Serial.println("Backward"); // Display motor direction
delay(2000); //2 seconds
}