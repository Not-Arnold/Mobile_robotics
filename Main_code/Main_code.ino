#define BUZZER_PIN 4   // use a safe GPIO

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 11
#define SCL_PIN 21

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Motor control pins
const int motor1PWM = 37;
const int motor1Phase = 38;
const int motor2PWM = 39;
const int motor2Phase = 20;

// Sensor pins
int AnalogPin[5] = {18, 17, 16, 15, 7};

// Array to keep track of values
int AnalogValue[5] = {0, 0, 0, 0, 0};
int last_speed[2] = {0,0};

// Marks the speed of each 
int currentLeftSpeed = 0;   // Add these here
int currentRightSpeed = 0;  // so loop() can see them

// Damping control and Sharp turning
const float Kp = 100; 
const float Ki = 0.0;
const float Kd = 80;

// Keeps track of error values
int lastError = 0;
float integral = 0;
const int baseSpeed = 255; 
const int maxSpeed = 255;

// Wifi code
const char* ssid     = "iot";
const char* password = "inflamedness65barra";

static int currentPosition = 0;   // ALWAYS start at 0
static bool finished = false;

const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";

void setup() {
  Serial.begin(115200);

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  // Sensor pin setup
  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);}

  // Wifi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);}

  // One immediate test post at startup
  String response = notifyArrival(0);
  if (response.length() > 0) {
    int nextDestination = response.toInt();}

  // OLED Display set up
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED NOT detected");
    while (true);}
}

void loop() {
  lineFollow();
  runClientLoop();
}

// Oled printing
void printOLED(const char* label, int value, uint8_t textSize) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s %d", label, value);

  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(buffer);
  display.display();}

// Wifi Code

// If server connection is successful
String notifyArrival(int position) {
  if (WiFi.status() != WL_CONNECTED) {
    return "";
  }

  String url = String(SERVER_BASE) + "/api/arrived/" + TEAM_ID;

  HTTPClient http;
  http.begin(url);

  String body = "position=" + String(position);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(body);

  String destination = "";

  if (httpCode > 0) {
    destination = http.getString();
    destination.trim();
  }

  http.end();
  return destination;}

// Calls the client
void runClientLoop() {
  static unsigned long lastSendMs = 0;
  const unsigned long SEND_INTERVAL_MS = 2000;

  if (finished) return;

  if (millis() - lastSendMs < SEND_INTERVAL_MS) return;
  lastSendMs = millis();

  // Notify server of arrival at CURRENT position
  String response = notifyArrival(currentPosition);

  if (response.length() == 0) {
    return;
  }

  if (response == "Finished") {
    finished = true;
    return;
  }

  //??????????????????
  int nextNode = response.toInt();
  currentPosition = nextNode;}


// Line follow code

// Line caller
void Line_Follow(){
  int error = calculateError();
  if (error == 100) {
    // Drives in the last direction if no line detected.
    driveMotors(last_speed[0], last_speed[1]);
    //stopMotors();

  } else {
    calculatePID(error);
  }}

// Calculates the how off the line we are.
int calculateError() {
  int threshold = 1000;
  
  for (int i = 0; i < 5; i++) {
    AnalogValue[i] = analogRead(AnalogPin[i]);
  }

  // Setting up the difference between a line and everything else
  bool s0 = AnalogValue[0] < threshold;
  bool s1 = AnalogValue[1] < threshold;
  bool s2 = AnalogValue[2] < threshold;
  bool s3 = AnalogValue[3] < threshold;
  bool s4 = AnalogValue[4] < threshold;

  // Node detection
  if (s0 && s1 && s2 && s3 && s4) {
    /*stopMotors();
    delay(10);
    nodeDetected++;
    printOLED("Node:", nodeDetected, 3);
    if (nodeDetected == 1){
      turningR();
      turningR();
    }
    if (nodeDetected == 6){
      turningR();
    } */}

  if (s0) return -2;
  if (s4) return 2;
  if (s1) return -1;
  if (s3) return 1;
  if (s2) return 0;

  return 100; }

// Calculates the values to change Kp and Kd
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
  last_speed[1] = currentRightSpeed;}

// Drives the motors at different speeds
void driveMotors(int left, int right) {
  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, right);}

// Turns at a junction or obstacle
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
  delay(200);}

// Motor stops
void stopMotors() {
  analogWrite(motor1PWM, 0);
  analogWrite(motor2PWM, 0);}

// Hard turn left
void turningL(){
  int turnSpeed = 200; // A manageable speed for rotating
  int threshold = 1000;

  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, turnSpeed);

  delay(300); 

  while (analogRead(AnalogPin[2]) > threshold) {}

  stopMotors();
  delay(200);}

// Hard turn right
void turningR(){
  int turnSpeed = 200; // A manageable speed for rotating
  int threshold = 1000;

  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, LOW); 
  analogWrite(motor2PWM, turnSpeed);

  delay(300); 

  while (analogRead(AnalogPin[2]) > threshold) {}

  stopMotors();
  delay(200);}

// rest of the code is for dancing
void dancing(){
  delay(1000);
  turning();
  delay(500);
  twerking(10);
  delay(100);
  wiggle(15);
  delay(100);}


void driveMotors_back(int left, int right) {
  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, right);}


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
  delay(100);}