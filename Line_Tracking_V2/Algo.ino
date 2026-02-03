void addEdge(int a, int b, int w, int headingAToB, int headingBToA) {
  nbr[a][deg[a]] = b;
  wgt[a][deg[a]] = w;
  edgeHeading[a][deg[a]] = headingAToB; // Heading when going a -> b
  deg[a]++;

  nbr[b][deg[b]] = a;
  wgt[b][deg[b]] = w;
  edgeHeading[b][deg[b]] = headingBToA; // Heading when going b -> a
  deg[b]++;
}

void buildGraph() {
  for (int i = 0; i < N; i++) deg[i] = 0;

  // Example: addEdge(NodeA, NodeB, Weight, Heading A->B, Heading B->A)
  // Curve 0 to 4
  addEdge(0, 4, 5, 3, 1); 
  
  // Junction 6 (Right side)
  addEdge(0, 6, 5, 0, 2);   // 0 up to 6 is North(0)
  addEdge(2, 6, 5, 2, 0);   // 2 down to 6 is South(2)
  addEdge(6, 1, 5, 3, 1);  // 6 left to 1 is West(3)

  // Junction 7 (Left side)
  addEdge(4, 7, 8, 0, 2);   // 4 up to 7 is North(0)
  addEdge(3, 7, 8, 2, 0);   // 3 down to 7 is South(2)
  addEdge(7, 1, 5, 1, 3);  // 7 right to 1 is East(1)
  
  // Curve 3 to 2
  addEdge(3, 2, 5, 1, 3); 
}


void getTurn(int pre, int cur, int next) {
  int h_in = -1;
  int h_out = -1;

  // 1. Find the heading we arrived with (pre -> cur)
  for (int i = 0; i < deg[pre]; i++) {
    if (nbr[pre][i] == cur) {
      h_in = edgeHeading[pre][i];
      break;
    }
  }

  // 2. Find the heading we need to leave with (cur -> next)
  for (int i = 0; i < deg[cur]; i++) {
    if (nbr[cur][i] == next) {
      h_out = edgeHeading[cur][i];
      break;
    }
  }

  int diff = (h_out - h_in + 4) % 4;

  if (diff == 0) direction = "STRAIGHT";
  else if (diff == 1) direction = "RIGHT";
  else if (diff == 2) { direction = "STRAIGHT"; uturn = true; } // Or custom U-turn
  else if (diff == 3) direction = "LEFT";
}


void nagvigating() {
  // 1. Get nodes from our pre-calculated Dijkstra path
  int pre = (pathIndex > 0) ? path[pathIndex - 1] : currentPosition;
  int cur = path[pathIndex];
  int next = path[pathIndex + 1];

  // 2. Determine the required turn based on headings
  getTurn(pre, cur, next);

  // 3. Execute the turn
  if (uturn) {
    turning();
    uturn = false;
  } else if (direction == "LEFT") {
    turningL();
  } else if (direction == "RIGHT") {
    turningR();
  } else if (direction == "STRAIGHT") {
    driveMotors(baseSpeed, baseSpeed);
    delay(300); // Increased to ensure the robot clears the junction line
  }

  // 4. Move to the next segment of the path
  pathIndex++; 
  lasttalktoserver = millis();
}



void findShortestPath(int startNode, int endNode) {
  int dist[N];
  int parent[N];
  bool visited[N];

  for (int i = 0; i < N; i++) {
    dist[i] = INF;
    parent[i] = -1;
    visited[i] = false;
  }

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
      if (dist[u] + weight < dist[v]) {
        dist[v] = dist[u] + weight;
        parent[v] = u;
      }
    }
  }

  // Reconstruct the path
  int tempPath[N];
  int tempIdx = 0;
  int curr = endNode;

  if (dist[endNode] == INF) return; 

  while (curr != -1) {
    tempPath[tempIdx++] = curr;
    curr = parent[curr];
  }

  // Fill the global path array in forward order
  for (int i = 0; i < tempIdx; i++) {
    path[i] = tempPath[tempIdx - 1 - i];
  }
  
  pathLength = tempIdx;
  pathIndex = 0; // CRITICAL: Reset the bookmark for the new journey
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

  stopMotors();
  delay(200);
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

  stopMotors();
  delay(200);
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

  stopMotors();
}

