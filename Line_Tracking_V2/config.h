#define BUZZER_PIN 4   // use a safe GPIO
#include <WiFi.h>
#include <HTTPClient.h>
#define PI 3.14159265
#include <math.h>
#include <ESP32Servo.h>

int motor1PWM = 35;
int motor1Phase = 37;
int motor2PWM = 38;
int motor2Phase = 21;

int AnalogPin[5] = {18, 17, 16, 15, 7};
int AnalogValue[5] = {0, 0, 0, 0, 0};
int last_speed[2] = {0,0};

int currentLeftSpeed = 0;   // Add these here
int currentRightSpeed = 0;  // so loop() can see them

float Kp = 25; 
float Ki = 0.0;
float Kd = 15;


int lastError = 0;
float integral = 0;
int baseSpeed = 200; 
int maxSpeed = 255;



unsigned long lastPrintTime = 0;
unsigned long lasttalktoserver = 0;



//wifi
const char* ssid = "iot";
const char* password = "inflamedness65barra";  // number 1
//const char* password = "repacks43telangiectases"; // number 2

static int currentPosition = 4;  
static bool finished = false;

const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";




// algo
bool firstRun = true;

const int N = 8;        // total vertices (0..N-1). Add junction nodes if needed.
const int MAX_DEG = 4;  // max neighbors per node (set >= your max degree)
const int INF = 1000000000;

int preNode;
int nextNode;
int curNode;
int previousNodeID = 4;
int globalTargetNode = -1;


int deg[N];           // number of neighbors for each node
int nbr[N][MAX_DEG];  // neighbor IDs
int wgt[N][MAX_DEG];  // edge weights to corresponding neighbor
int edgeHeading[N][MAX_DEG]; 

int path[N];
int pathLength = 0;
int pathIndex = 0;


//obst
int TRIG_PIN = 19;
int ECHO_PIN = 2;


int SERVO_PIN = 1;

Servo myServo;
const int servoCentre = 90;
const int servoMin = 35;
const int servoMax = 145;
float servoScale = 1.0;