int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;

int speed = 200;
float ratio = 0.93;

void setup() {
  Serial.begin(9600);
  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);
  }

void loop() {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, speed*ratio); 
  Serial.println("Forward"); 
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, speed); 
  Serial.println("Forward"); 
  delay(2000);

  digitalWrite(motor1Phase, LOW);
  analogWrite(motor1PWM, speed*ratio);
  Serial.println("Backward");
  digitalWrite(motor2Phase, HIGH);
  analogWrite(motor2PWM, speed);
  Serial.println("Backward");
  delay(2000);
  }

