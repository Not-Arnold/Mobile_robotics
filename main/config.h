#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define PI 3.14159265
#include <math.h>
#include <ESP32Servo.h>

int motor1PWM = 35;
int motor1Phase = 37;
int motor2PWM = 38;
int motor2Phase = 21;

int TRIG_PIN = 19;
int ECHO_PIN = 2;

int SERVO_PIN = 1;

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
int nextNode = 0;
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


Servo myServo;
const int servoCentre = 90;
const int servoMin = 20;
const int servoMax = 160;
float servoScale = 1.0;

// Update this from your cloudflared terminal
String BASE_URL = "https://vendors-hugh-estimation-telecharger.trycloudflare.com";

const char* API_TOKEN = "twiningESP";
String ROBOT_ID = "asun2881"; 

uint32_t lastTelemetryTime = 500; 
// ------------------------------------------------

// --- GLOBAL STATE VARIABLES (Testing Base) ---
int currentNode = 4;      // Car icon will start at Node 1
int nextnode    = 0;      // Target circle will start at Node 6
bool obstacle   = false;
int obstacleStartNode = -1;
int obstacleEndNode = -1;
String stateStr = "IDLE";
String routeStr = "4 -> ";