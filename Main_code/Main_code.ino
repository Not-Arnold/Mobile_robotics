#define BUZZER_PIN 4   // use a safe GPIO

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 11
#define SCL_PIN 21

//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
//const char* password = "inflamedness65barra"; // number 1
const char* password = "repacks43telangiectases"; // number 2

static int currentPosition = 0;   // ALWAYS start at 0
static bool finished = false;

const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";

const int maxDestinations = 19;
int destinations[maxDestinations] = {0};
int destCount = 1;

const int N = 8;              // total vertices (0..N-1). Add junction nodes if needed.
const int MAX_DEG = 4;        // max neighbors per node (set >= your max degree)
const int INF = 1000000000;

int deg[N];                   // number of neighbors for each node
int nbr[N][MAX_DEG];          // neighbor IDs
int wgt[N][MAX_DEG];          // edge weights to corresponding neighbor

int route[32];          // will hold nodes e.g. 0 -> 6 -> 2 -> 1
int routeLen = 0;       // how many entries valid in route[]
int routeIdx = 0;       // current waypoint index inside route[]
int goalNode = -1;

void setup() {
  Serial.begin(115200);

  pinMode(motor1PWM, OUTPUT);
  pinMode(motor1Phase, OUTPUT);
  pinMode(motor2PWM, OUTPUT);
  pinMode(motor2Phase, OUTPUT);

  // Sensor pin setup
  for (int i = 0; i < 5; i++) {
    pinMode(AnalogPin[i], INPUT);}

  stopMotors();

  // Wifi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected");
    delay(100);}
  
  Serial.println("connected");

  // One immediate test post at startup
  String response = notifyArrival(5);
  if (response.length() > 0) {
    int nextDestination = response.toInt();}

  buildGraph();

  /*// OLED Display set up
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED NOT detected");
    while (true);}*/
}

bool print = false;

void loop() {
  while (!finished){
    runClientLoop();}
  if (!print){
    //printDestinations();
    computeRoutesForDestinationPairs();
    print = true;}
  lineFollow();
}

// Oled printing
/*void printOLED(const char* label, int value, uint8_t textSize) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s %d", label, value);

  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(buffer);
  display.display();}*/

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
  if (finished) return;

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
  currentPosition = nextNode;
  destinations[destCount++] = nextNode;}

// Line follow code

// Path finding code
/ ------------------------------
// Example setup using a sample graph
// (Replace with your real node connections + weights)
// ------------------------------
void buildGraph() {
  for (int i = 0; i < N; i++) deg[i] = 0;

  // Example edges (YOU must set these for your map)
  // addEdge(u, v, weight);

  addEdge(0, 6, 10); // 6 could be right junction, etc.
  addEdge(6, 2, 10);
  addEdge(2, 3, 10);
  addEdge(3, 7, 15);
  addEdge(7, 4, 15);
  addEdge(4, 0, 10);
  addEdge(6, 1, 5);
  addEdge(1, 7, 5);

  addEdge(7, 5, 40); // makes node 5 reachable in this example
} 
void addEdge(int a, int b, int w) {
  nbr[a][deg[a]] = b;
  wgt[a][deg[a]] = w;
  deg[a]++;

  nbr[b][deg[b]] = a;
  wgt[b][deg[b]] = w;
  deg[b]++;}

void dijkstra(int start, int dist[], int prev[]) {
  bool used[N];

  // init
  for (int i = 0; i < N; i++) {
    dist[i] = INF;
    prev[i] = -1;
    used[i] = false;
  }
  dist[start] = 0;

  // main loop: pick closest unused node each time
  for (int iter = 0; iter < N; iter++) {
    int u = -1;
    int best = INF;

    // find unused node with smallest dist
    for (int i = 0; i < N; i++) {
      if (!used[i] && dist[i] < best) {
        best = dist[i];
        u = i;
      }
    }

    if (u == -1) break; // remaining nodes unreachable
    used[u] = true;

    // relax edges out of u
    for (int k = 0; k < deg[u]; k++) {
      int v = nbr[u][k];
      int w = wgt[u][k];
      if (w < 0) continue; // (Dijkstra requires non-negative weights)

      long alt = (long)dist[u] + (long)w;
      if (alt < dist[v]) {
        dist[v] = (int)alt;
        prev[v] = u;
      }
    }
  }}
// Reconstruct path start->target using prev[]
// Returns path length, writes nodes into pathOut[0..len-1]
int buildPath(int target, const int prev[], int pathOut[], int maxLen) {
  int len = 0;
  int cur = target;

  while (cur != -1 && len < maxLen) {
    pathOut[len++] = cur;
    cur = prev[cur];
  }

  // reverse
  for (int i = 0; i < len / 2; i++) {
    int tmp = pathOut[i];
    pathOut[i] = pathOut[len - 1 - i];
    pathOut[len - 1 - i] = tmp;
  }
  return len;}



bool computeRouteTo(int start, int goal) {
  if (goal < 0 || goal >= N) return false;

  int dist[N], prev[N];
  dijkstra(start, dist, prev);

  if (dist[goal] >= INF/2) {
    // unreachable
    routeLen = 0;
    routeIdx = 0;
    return false;
  }

  routeLen = buildPath(goal, prev, route, (int)(sizeof(route)/sizeof(route[0])));
  routeIdx = 0;
  goalNode = goal;

  // Debug print
  Serial.print("New route to "); Serial.print(goal);
  Serial.print(" (len="); Serial.print(routeLen); Serial.println("):");
  for (int i = 0; i < routeLen; i++) {
    Serial.print(route[i]);
    if (i < routeLen - 1) Serial.print(" -> ");
  }
  Serial.println();

  return true;}



void computeRoutesForDestinationPairs() {
  for (int i = 0; i < destCount - 1; i++) {
    int start = destinations[i];
    int goal  = destinations[i+1];
    computeRouteTo(start, goal);   // prints the route
  }}

// Line caller
void lineFollow(){
  int error = calculateError();
  if (error == 100) {
    // Drives in the last direction if no line detected.
    driveMotors(last_speed[0], last_speed[1]);
    //stopMotors();

  } else {
    calculatePID(error);
  }}
//
int getCurrNode() {
  if (routeLen == 0) return -1;
  return route[routeIdx];
}

int getPrevNode() {
  if (routeLen == 0 || routeIdx == 0) return -1;
  return route[routeIdx - 1];
}

int getNextNode() {
  if (routeLen == 0 || routeIdx >= routeLen - 1) return -1;
  return route[routeIdx + 1];
}


void nodeEvent(){
  static unsigned long lastHit = 0;
  if (millis() - lastHit < 400) return;

  if (routeLen > 0 && routeIdx == routeLen - 1){
    int prev = getPrevNode();
    int curr = getCurrNode();
    int next = getNextNode();

    String doTurn = getTurn(prev, curr, next);
    if (doTurn == "STRAIGHT") return;
    else if (doTurn == "RIGHT") {turningR(); return;}
    else if (doTurn == "LEFT") {turningL(); return;}
    else {turningR(); return;}
    }
}

// Turn logic
String getTurn(int prev, int curr, int next){
  if (next == prev) return "UTURN";

  if (curr == 6){
    if (prev == 0 && next == 1) return "LEFT";
    if (prev == 0 && next == 2) return "STRAIGHT";

    if (prev == 2 && next == 1) return "RIGHT";
    if (prev == 2 && next == 0) return "STRAIGHT";

    if (prev == 1 && next == 2) return "LEFT";
    if (prev == 1 && next == 0) return "RIGHT";
  }

  if (curr == 7){
    if (prev == 3 && next == 1) return "LEFT";
    if (prev == 3 && next == 4) return "STRAIGHT";

    if (prev == 4 && next == 1) return "RIGHT";
    if (prev == 4 && next == 3) return "STRAIGHT";

    if (prev == 1 && next == 4) return "LEFT";
    if (prev == 1 && next == 3) return "RIGHT";
  }

  return "STRAIGHT";
}



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
    nodeEvent();
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
  turningR();
  delay(500);
  nodding(10);
  delay(100);
  wiggle(15);
  delay(100);}


void driveMotors_back(int left, int right) {
  digitalWrite(motor1Phase, LOW); 
  analogWrite(motor1PWM, left);

  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, right);}


void nodding(int times) {
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