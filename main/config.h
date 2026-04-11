// ============================================================
// config.h — Global Configuration, Pins, Constants & State
//
// Central place for:
// 1. Library includes
// 2. Hardware pin assignments
// 3. PID and motion constants
// 4. Wi-Fi / server credentials
// 5. Graph/pathfinding storage
// 6. Servo and telemetry state
// 7. Shared global runtime variables
// ============================================================

#include <WiFi.h>              // Wi-Fi connection for server/dashboard comms
#include <HTTPClient.h>        // HTTP requests
#include <WiFiClientSecure.h>  // HTTPS client support

#define PI 3.14159265
#include <math.h>
#include <ESP32Servo.h>


// -------------------------------------------------------
// MOTOR DRIVER PINS
// motorXPWM   = speed control pin
// motorXPhase = direction control pin
// -------------------------------------------------------
int motor1PWM = 35;
int motor1Phase = 37;
int motor2PWM = 38;
int motor2Phase = 21;


// -------------------------------------------------------
// ULTRASONIC SENSOR PINS
// TRIG_PIN sends the pulse, ECHO_PIN receives the return
// -------------------------------------------------------
int TRIG_PIN = 19;
int ECHO_PIN = 2;


// -------------------------------------------------------
// SERVO PIN
// Used for steering / sensor positioning depending on setup
// -------------------------------------------------------
int SERVO_PIN = 1;


// -------------------------------------------------------
// LINE SENSOR ARRAY
// AnalogPin[] holds the GPIOs connected to the 5 sensors
// AnalogValue[] stores the latest readings from those sensors
// -------------------------------------------------------
int AnalogPin[5] = {18, 17, 16, 15, 7};
int AnalogValue[5] = {0, 0, 0, 0, 0};

// Last commanded motor speeds, useful for debugging or smoothing
int last_speed[2] = {0,0};


// -------------------------------------------------------
// CURRENT MOTOR OUTPUT STATE
// These are updated so other files/functions (like telemetry)
// can report the robot's live motor speeds
// -------------------------------------------------------
int currentLeftSpeed = 0;   // Current left motor output for control/telemetry
int currentRightSpeed = 0;  // Current right motor output for control/telemetry


// -------------------------------------------------------
// PID CONTROL CONSTANTS
// Used by the line-following controller
// -------------------------------------------------------
float Kp = 25; 
float Ki = 0.0;
float Kd = 15;


// PID internal state
int lastError = 0;
float integral = 0;

// Base motion settings
int baseSpeed = 200; 
int maxSpeed = 255;


// -------------------------------------------------------
// SERVER TIMING
// Tracks the last time the robot contacted the server
// -------------------------------------------------------
unsigned long lasttalktoserver = 0;


// -------------------------------------------------------
// WI-FI CREDENTIALS
// Uncomment the correct password for the active network
// -------------------------------------------------------
const char* ssid = "iot";
const char* password = "inflamedness65barra";  // number 1
//const char* password = "repacks43telangiectases"; // number 2


// -------------------------------------------------------
// GLOBAL RUN STATUS
// currentPosition = robot's current node ID on the map
// finished        = true when all deliveries/parking are done
// -------------------------------------------------------
static int currentPosition = 4;  
static bool finished = false;


// -------------------------------------------------------
// COMPETITION / SERVER IDENTITY
// TEAM_ID is used with the competition server
// SERVER_BASE is the base URL for arrival notifications
// -------------------------------------------------------
const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";


// -------------------------------------------------------
// PATHFINDING / ROUTING STATE
// firstRun tells nodeEvent() whether the robot has not yet
// requested its first destination from the server
// -------------------------------------------------------
bool firstRun = true;


// -------------------------------------------------------
// GRAPH CONFIGURATION
// N        = total number of nodes in the map
// MAX_DEG  = maximum number of neighbours any node can have
// INF      = effectively "blocked" or unreachable edge cost
// -------------------------------------------------------
const int N = 8;        // total vertices (0..N-1). Add junction nodes if needed.
const int MAX_DEG = 4;  // max neighbors per node (set >= your max degree)
const int INF = 1000000000;


// -------------------------------------------------------
// NODE-TO-NODE NAVIGATION VARIABLES
// preNode / curNode / nextNode may be used during traversal
// previousNodeID stores the direction the robot came from
// globalTargetNode stores the final destination currently assigned
// -------------------------------------------------------
int preNode;
int nextNode = 0;
int curNode;
int previousNodeID = 4;
int globalTargetNode = -1;


// -------------------------------------------------------
// GRAPH STORAGE
// deg[i]      = number of neighbours of node i
// nbr[i][j]   = j-th neighbour of node i
// wgt[i][j]   = weight/cost of edge from i to nbr[i][j]
// edgeHeading = reserved/extra heading data if needed later
// -------------------------------------------------------
int deg[N];           // number of neighbors for each node
int nbr[N][MAX_DEG];  // neighbor IDs
int wgt[N][MAX_DEG];  // edge weights to corresponding neighbor
int edgeHeading[N][MAX_DEG]; 


// -------------------------------------------------------
// SHORTEST PATH STORAGE
// path[]     = computed route from current node to target
// pathLength = number of valid entries in path[]
// pathIndex  = current progress index along the route
// -------------------------------------------------------
int path[N];
int pathLength = 0;
int pathIndex = 0;


// -------------------------------------------------------
// SERVO CONFIGURATION
// servoCentre = neutral angle
// servoMin    = minimum allowed servo angle
// servoMax    = maximum allowed servo angle
// servoScale  = optional scaling factor for servo movement
// -------------------------------------------------------
Servo myServo;
const int servoCentre = 90;
const int servoMin = 20;
const int servoMax = 160;
float servoScale = 1.0;


// -------------------------------------------------------
// DASHBOARD / TELEMETRY SERVER
// BASE_URL should be updated whenever the Cloudflare tunnel URL changes
// -------------------------------------------------------
String BASE_URL = "https://vendors-hugh-estimation-telecharger.trycloudflare.com";

const char* API_TOKEN = "twiningESP";
String ROBOT_ID = "asun2881"; 

// -------------------------------------------------------
// TELEMETRY TIMING
// Used to limit how often telemetry is sent
// -------------------------------------------------------
uint32_t lastTelemetryTime = 500; 



// -------------------------------------------------------
// DASHBOARD DISPLAY STATE
// These variables are mainly used for the live web dashboard
//
// currentNode       = node currently shown for the robot icon
// nextnode          = next target node shown on the dashboard
// obstacle          = whether an obstacle is currently flagged
// obstacleStartNode / obstacleEndNode = blocked edge endpoints
// stateStr          = current robot state text
// routeStr          = route shown in the dashboard
// -------------------------------------------------------
int currentNode = 4;      // Car icon will start at Node 1
int nextnode    = 0;      // Target circle will start at Node 6
bool obstacle   = false;
int obstacleStartNode = -1;
int obstacleEndNode = -1;
String stateStr = "IDLE";
String routeStr = "4 -> ";
