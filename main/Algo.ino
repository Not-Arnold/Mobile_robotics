struct Point {
  float x;
  float y;
};

// Physical Map (X, Y Coordinates)
// This allows the robot to "see" the shape of the path
Point nodes[N] = {
  { 5.0, -8.7}, // ID 0 (Bottom Right)
  { 0.0,  0.0}, // ID 1 (Center)
  { 5.0,  8.7}, // ID 2 (Top Right)
  {-5.0,  8.7}, // ID 3 (Top Left)
  {-5.0, -8.7}, // ID 4 (Bottom Left)
  { 0.0,  0.0}, // ID 5 (UNUSED)
  {10.0,  0.0}, // ID 6 (Right)
  {-10.0, 0.0}  // ID 7 (Left)
};

// Simplified addEdge: No need to manually type headings!
void addEdge(int u, int v, int w) {
  // Add U -> V
  nbr[u][deg[u]] = v;
  wgt[u][deg[u]] = w;
  deg[u]++;

  // Add V -> U (Bidirectional)
  nbr[v][deg[v]] = u;
  wgt[v][deg[v]] = w;
  deg[v]++;
}


void buildGraph() {
  // Clear old connections
  for (int i = 0; i < N; i++) deg[i] = 0;
  
  // 1. Center Line
  addEdge(1, 6, 10);
  addEdge(1, 7, 10);
  addEdge(0, 6, 10);
  addEdge(6, 2, 10);
  addEdge(2, 3, 10);
  addEdge(3, 7, 15);
  addEdge(7, 4, 15);
  addEdge(4, 0, 10);
}



String getTurn(int prev, int curr, int next) {
  // 1. U-Turn Check
  if (prev == next) return "U-TURN";

  // 2. Get Coordinates
  Point p1 = nodes[prev];
  Point p2 = nodes[curr];
  Point p3 = nodes[next];

  // 3. Calculate Angles
  float angleIn = atan2(p2.y - p1.y, p2.x - p1.x);
  float angleOut = atan2(p3.y - p2.y, p3.x - p2.x);
  
  float diff = angleOut - angleIn;
  
  // Normalize (-PI to +PI)
  while (diff <= -PI) diff += 2 * PI;
  while (diff > PI) diff -= 2 * PI;

  // Convert to Degrees
  float deg = diff * 180.0 / PI;

  // 4. Decide Direction
  // Wide window (-65 to +65) handles the gentle curve of the circle
  if (deg > -65 && deg < 65) return "STRAIGHT";
  if (deg >= 65) return "LEFT";
  return "RIGHT";
}


void navigating() {
  int currentNode = path[pathIndex];
  int pre = (pathIndex > 0) ? path[pathIndex - 1] : previousNodeID;
  int cur = path[pathIndex];
  int next = path[pathIndex + 1];

  // --- FIX START ---
  // 1. Capture the result!
  String action = getTurn(pre, cur, next); 

  // 2. Use 'action' variable instead of global strings/bools
  if (action == "U-TURN") {
    turning();
    // No need to reset a global flag anymore
  } else if (action == "LEFT") {
    turningL();
  } else if (action == "RIGHT") {
    turningR();
  } else if (action == "STRAIGHT") {
    driveMotors(baseSpeed, baseSpeed);
    delay(150);
  }
  // --- FIX END ---

  pathIndex++;
  lasttalktoserver = millis();
}



void findShortestPath(int startNode, int endNode) {
  int dist[N]; int parent[N]; bool visited[N];
  for (int i = 0; i < N; i++) { dist[i] = INF; parent[i] = -1; visited[i] = false; } 

  dist[startNode] = 0;
  for (int count = 0; count < N - 1; count++) {
    int u = -1;
    for (int i = 0; i < N; i++) {
      if (!visited[i] && (u == -1 || dist[i] < dist[u])) u = i; 
    }
    if (u == -1 || dist[u] == INF) break; 
    visited[u] = true;

    for (int i = 0; i < deg[u]; i++) {
      int v = nbr[u][i]; int weight = wgt[u][i]; 
      if (dist[u] + weight < dist[v]) { dist[v] = dist[u] + weight; parent[v] = u; } 
    }
  }

  int tempPath[N]; int tempIdx = 0; int curr = endNode; 
  if (dist[endNode] == INF) return;
  while (curr != -1) { tempPath[tempIdx++] = curr; curr = parent[curr]; } 
  for (int i = 0; i < tempIdx; i++) { path[i] = tempPath[tempIdx - 1 - i]; } 
  
  pathLength = tempIdx; // [cite: 39]
  pathIndex = 0; // Reset bookmark for new journey [cite: 39]
}

void turningL() {
  int turnSpeed = 200;  // A manageable speed for rotating
  int threshold = 500;

  digitalWrite(motor1Phase, HIGH);
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH);
  analogWrite(motor2PWM, turnSpeed);

  delay(250);

  while ((analogRead(AnalogPin[2]) > threshold) && (analogRead(AnalogPin[1]) > threshold) && (analogRead(AnalogPin[3]) > threshold)){}
}

// Hard turn right
void turningR() {
  int turnSpeed = 200;  // A manageable speed for rotating
  int threshold = 500;

  digitalWrite(motor1Phase, LOW);
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, LOW);
  analogWrite(motor2PWM, turnSpeed);

  delay(250);

  while ((analogRead(AnalogPin[2]) > threshold) && (analogRead(AnalogPin[1]) > threshold) && (analogRead(AnalogPin[3]) > threshold)){}
}

void turning(){
  int turnSpeed = 200; // A manageable speed for rotating
  int threshold = 500;

  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, turnSpeed);

  delay(700); 

  while ((analogRead(AnalogPin[2]) > threshold) && (analogRead(AnalogPin[1]) > threshold) && (analogRead(AnalogPin[3]) > threshold)){}
}