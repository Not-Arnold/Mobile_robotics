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
  {-15.0, 0.0}, // ID 5 (Parking - To the left of Node 7)
  {10.0,  0.0}, // ID 6 (Right)
  {-10.0, 0.0}  // ID 7 (Left)
};

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

void blockEdge(int u, int v) {
  // Block U -> V
  for (int i = 0; i < deg[u]; i++) {
    if (nbr[u][i] == v) {
      wgt[u][i] = INF; // Set weight to infinity
      break;
    }
  }
  
  // Block V -> U (Bidirectional)
  for (int i = 0; i < deg[v]; i++) {
    if (nbr[v][i] == u) {
      wgt[v][i] = INF;
      break;
    }
  }
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

  addEdge(7, 5, 1000);
}

String getTurn(int prev, int curr, int next) {
  // 1. U-Turn Check
  if (prev == next) return "U-TURN";

  // 1. Special Parking Trigger
  if (curr == 7 && next == 5) return "PARK";
  if (prev == 7 && curr == 1 && next == 7) return "SOFT_U_TURN";

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

  // 1. Capture the result!
  String action = getTurn(pre, cur, next); 

  // 2. Use 'action' variable instead of global strings/bools
  if (action == "U-TURN") {
    turning();
  } else if (action == "SOFT_U_TURN") {
    softturning();
  } else if (action == "LEFT") {
    turningL();
  } else if (action == "RIGHT") {
    turningR();
  } else if (action == "PARK") {     
    driveStraightToParking();
    finished = true;
  } else if (action == "STRAIGHT") {
    driveMotors(baseSpeed*0.9, baseSpeed);
    delay(500);
  }

  pathIndex++;
}

void findShortestPath(int startNode, int endNode) {
  int actualEndNode = endNode;
  
  // SPECIAL CASE: If going to 5, we must ALWAYS go through 1 then 7.
  // This ensures we are aligned straight, even if we start at 7.
  // So, we tell the pathfinder to take us to Node 1 first.
  if (endNode == 5) {
      actualEndNode = 1;
  }

  // --- Standard Dijkstra Algorithm ---
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
      int v = nbr[u][i];
      int weight = wgt[u][i];
      if (dist[u] + weight < dist[v]) { dist[v] = dist[u] + weight; parent[v] = u; }
    }
  }

  // Reconstruct path from Start -> actualEndNode
  int tempPath[N]; int tempIdx = 0; int curr = actualEndNode;
  if (dist[actualEndNode] == INF) return;
  while (curr != -1) { tempPath[tempIdx++] = curr; curr = parent[curr]; }
  
  for (int i = 0; i < tempIdx; i++) { path[i] = tempPath[tempIdx - 1 - i]; }
  
  pathLength = tempIdx; 

  // --- APPEND THE FINAL APPROACH ---
  // If the target was 5, we are currently at Node 1 (or at Node 7 heading to 1).
  // Now we append the sequence 1 -> 7 -> 5 to the path.
  if (endNode == 5) {
      path[pathLength] = 7;
      path[pathLength + 1] = 5;
      pathLength += 2;
  }

  updateRouteStringProgress();

  pathIndex = 0;
}

void updateRouteStringProgress() {
  routeStr = "";
  for (int i = 0; i < pathLength; i++) {
    if (i == pathIndex) {
      routeStr += "[" + String(path[i]) + "]"; // Put brackets around current target
    } else {
      routeStr += String(path[i]);
    }
    if (i < pathLength - 1) routeStr += " -> ";
  }
}

void nodeEvent(){
  int arrivalNode;

  if (firstRun) {
    arrivalNode = 0; // Or 4, wherever you physically place the robot
  } else {
    arrivalNode = path[pathIndex];
    currentPosition = arrivalNode;
  }

  Serial.print("Arrived at ID: ");
  Serial.println(arrivalNode);

  // 2. Check if we are at the end of the current route
  bool isFinalDestination = (pathIndex == pathLength - 1);

  if (firstRun || isFinalDestination) {

    if (!firstRun) {
          previousNodeID = path[pathIndex - 1]; 
      } else {
          previousNodeID = 4; // Default for first run
      }

      // Talk to Server
      String response = notifyArrival(arrivalNode);
      
      if (response == "Finished" || response == "") {
        finished = true;
        return;
      }

      int targetNode = response.toInt();
      globalTargetNode = targetNode;
      Serial.print("Server says go to: ");
      Serial.println(targetNode);

      findShortestPath(arrivalNode, targetNode);
      
      firstRun = false;

      navigating();
    }

  else {
    // --- WE ARE AT AN INTERMEDIATE NODE (6 or 7) ---
    // Do not talk to server. Just turn and keep driving.
    Serial.println("Intermediate Node - Keep Going");
    navigating();
  }
}

// The Reroute Routine
void performReroute() {
  stopMotors();
  Serial.println("Obstacle! Rerouting...");

  // 1. Identify where we are
  // We were going from path[pathIndex-1] (Node 0) -> path[pathIndex] (Node 4)
  // Note: Ensure pathIndex > 0 safety check
  int lastSafeNode = path[pathIndex - 1]; 
  int blockedNode = path[pathIndex];

  // 2. Perform Physical U-Turn
  // This physically points the robot back toward 'lastSafeNode' (Node 0)
  turning(); // Or a custom 180 turn function

  // After path[] and pathLength are updated:
  updateRouteStr(); 
  
  // Force a telemetry send so the dashboard updates the green text box instantly
  sendTelemetry();

  stateStr = "RE-ROUTING";

  // 3. Block the edge in software so Dijkstra won't use it again
  blockEdge(lastSafeNode, blockedNode);

  // 4. Re-calculate path from the node we are heading back to
  findShortestPath(lastSafeNode, globalTargetNode);

  // 5. CRITICAL STATE UPDATE
  // We are now driving TOWARD 'lastSafeNode' (Node 0).
  // Dijkstra sets path[0] = lastSafeNode.
  pathIndex = 0; 

  // TRICK: When we arrive at Node 0, the 'getTurn' function will ask:
  // "Where did I come from?" (prev), "Where am I?" (curr), "Where do I go?" (next).
  // We physically came from the blocked node direction.
  // So we manually set previousNodeID to the blocked node.
  previousNodeID = blockedNode;
  
  // Resume loop; the PID will now drive us to path[0] (Node 0).
}

void updateRouteStr() {
  routeStr = "";
  for (int i = 0; i < pathLength; i++) {
    routeStr += String(path[i]);
    if (i < pathLength - 1) routeStr += " -> ";
  }
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

void softturning(){
  int turnSpeed = 50; // A manageable speed for rotating
  int threshold = 500;

  digitalWrite(motor1Phase, HIGH); 
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH); 
  analogWrite(motor2PWM, turnSpeed);

  delay(1200); 

  while (analogRead(AnalogPin[2]) > threshold){}
}