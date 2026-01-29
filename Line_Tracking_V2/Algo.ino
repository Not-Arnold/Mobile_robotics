void addEdge(int a, int b, int w) {
  nbr[a][deg[a]] = b;
  wgt[a][deg[a]] = w;
  deg[a]++;

  nbr[b][deg[b]] = a;
  wgt[b][deg[b]] = w;
  deg[b]++;
}


void buildGraph() {
  for (int i = 0; i < N; i++) deg[i] = 0;

  // Example edges (YOU must set these for your map)
  // addEdge(u, v, weight);

  addEdge(0, 6, 10);  // 6 could be right junction, etc.
  addEdge(6, 2, 10);
  addEdge(2, 3, 10);
  addEdge(3, 7, 15);
  addEdge(7, 4, 15);
  addEdge(4, 0, 10);
  addEdge(6, 1, 5);
  addEdge(1, 7, 5);

  addEdge(7, 5, 40);  // makes node 5 reachable in this example
}



String getTurn(int prev, int curr, int next) {
  if (next == prev) return "UTURN";

  if (curr == 6) {
    if (prev == 0 && next == 1) return "LEFT";
    if (prev == 0 && next == 2) return "STRAIGHT";

    if (prev == 2 && next == 1) return "RIGHT";
    if (prev == 2 && next == 0) return "STRAIGHT";

    if (prev == 1 && next == 2) return "LEFT";
    if (prev == 1 && next == 0) return "RIGHT";
  }

  if (curr == 7) {
    if (prev == 3 && next == 1) return "LEFT";
    if (prev == 3 && next == 4) return "STRAIGHT";

    if (prev == 4 && next == 1) return "RIGHT";
    if (prev == 4 && next == 3) return "STRAIGHT";

    if (prev == 1 && next == 4) return "LEFT";
    if (prev == 1 && next == 3) return "RIGHT";
  }

  return "STRAIGHT";
}

void turningL() {
  int turnSpeed = 200;  // A manageable speed for rotating
  int threshold = 1000;

  digitalWrite(motor1Phase, HIGH);
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH);
  analogWrite(motor2PWM, turnSpeed);

  delay(300);

  while (analogRead(AnalogPin[2]) > threshold) {}

  stopMotors();
  delay(200);
}

// Hard turn right
void turningR() {
  int turnSpeed = 200;  // A manageable speed for rotating
  int threshold = 1000;

  digitalWrite(motor1Phase, LOW);
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, LOW);
  analogWrite(motor2PWM, turnSpeed);

  delay(300);

  while (analogRead(AnalogPin[2]) > threshold) {}

  stopMotors();
  delay(200);
}