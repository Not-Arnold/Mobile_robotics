#define BUZZER_PIN 4   // use a safe GPIO
#include <WiFi.h>
#include <HTTPClient.h>

int motor1PWM = 37;
int motor1Phase = 38;
int motor2PWM = 39;
int motor2Phase = 20;

int AnalogPin[5] = {18, 17, 16, 15, 7};
int AnalogValue[5] = {0, 0, 0, 0, 0};
int last_speed[2] = {0,0};

int currentLeftSpeed = 0;   // Add these here
int currentRightSpeed = 0;  // so loop() can see them

float Kp = 100; 
float Ki = 0.0;
float Kd = 80;


int lastError = 0;
float integral = 0;
int baseSpeed = 200; 
int maxSpeed = 255;

unsigned long lastPrintTime = 0;
unsigned long lasttalktoserver = 0;

//wifi
const int maxDestinations = 100;
int destinations[maxDestinations] = {4, 0};
int destCount = 1;

const char* ssid = "iot";
const char* password = "inflamedness65barra";  // number 1
//const char* password = "repacks43telangiectases"; // number 2

static int currentPosition = 0;  // ALWAYS start at 0
static bool finished = false;

const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";


// algo
const int N = 8;        // total vertices (0..N-1). Add junction nodes if needed.
const int MAX_DEG = 4;  // max neighbors per node (set >= your max degree)
const int INF = 1000000000;
int nextturn = 0;

int preNode;
int nextNode;
int curNode;
String direction;
bool uturn;

int deg[N];           // number of neighbors for each node
int nbr[N][MAX_DEG];  // neighbor IDs
int wgt[N][MAX_DEG];  // edge weights to corresponding neighbor