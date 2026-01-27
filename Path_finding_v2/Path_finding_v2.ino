// ------------------------------
// Simple Dijkstra for ESP32/Arduino
// - fixed-size arrays
// - O(N^2) (fine for small N)
// - supports positive weights
// ------------------------------

#include <WiFi.h>
#include <HTTPClient.h>

const int N = 8;              // total vertices (0..N-1). Add junction nodes if needed.
const int MAX_DEG = 4;        // max neighbors per node (set >= your max degree)
const int INF = 1000000000;

int deg[N];                   // number of neighbors for each node
int nbr[N][MAX_DEG];          // neighbor IDs
int wgt[N][MAX_DEG];          // edge weights to corresponding neighbor

// Wifi code
const char* ssid     = "iot";
const char* password = "repacks43telangiectases";

static int currentPosition = 0;   // ALWAYS start at 0
static bool finished = false;

const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";

const int maxDestinations = 20;
int destinations[maxDestinations] = {0};
int destCount = 1;

// Add undirected edge a<->b with weight w
void addEdge(int a, int b, int w) {
  nbr[a][deg[a]] = b;
  wgt[a][deg[a]] = w;
  deg[a]++;

  nbr[b][deg[b]] = a;
  wgt[b][deg[b]] = w;
  deg[b]++;
}

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
  }
}

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
  return len;
}

// Example: print dist + path to each node
void printAllPaths(int start) {
  int dist[N], prev[N];
  dijkstra(start, dist, prev);

  for (int t = 0; t < N; t++) {
    Serial.print("To ");
    Serial.print(t);
    Serial.print(" dist=");

    if (dist[t] >= INF/2) {
      Serial.println("INF (unreachable)");
      continue;
    }

    Serial.println(dist[t]);

    int path[32];
    int len = buildPath(t, prev, path, 32);

    Serial.print("  path: ");
    for (int i = 0; i < len; i++) {
      Serial.print(path[i]);
      if (i < len - 1) Serial.print(" -> ");
    }
    Serial.println();
  }
}

// ------------------------------
// Example setup using a sample graph
// (Replace with your real node connections + weights)
// ------------------------------
void buildGraphExample() {
  for (int i = 0; i < N; i++) deg[i] = 0;

  // Example edges (YOU must set these for your map)
  // addEdge(u, v, weight);

  addEdge(0, 6, 10); // 6 could be right junction, etc.
  addEdge(6, 2, 10);
  addEdge(2, 3, 10);
  addEdge(3, 7, 15);
  addEdge(7, 4, 15);
  //addEdge(3, 4, 10);
  addEdge(4, 0, 10);
  
  addEdge(6, 1, 5);
  addEdge(1, 7, 5);

  addEdge(7, 5, 40); // makes node 5 reachable in this example
}

// If server connection is successful
String notifyArrival(int position) {
  if (WiFi.status() != WL_CONNECTED) {
    return "";
  }

  Serial.println("testing");
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
  Serial.println(destination);
  http.end();
  return destination;}

// Calls the client
void runClientLoop() {
  static unsigned long lastSendMs = 0;
  const unsigned long SEND_INTERVAL_MS = 600;

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
  currentPosition = nextNode;
  destinations[destCount++] = nextNode;}


void setup() {
  Serial.begin(115200);
  buildGraphExample();
  printAllPaths(1);

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
}

int route[32];          // will hold nodes e.g. 0 -> 6 -> 2 -> 1
int routeLen = 0;       // how many entries valid in route[]
int routeIdx = 0;       // current waypoint index inside route[]
int goalNode = -1;

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

  return true;
}

// Returns next node robot should drive toward (waypoint).
// If already at end, returns -1.
int getNextWaypoint() {
  // route[0] should be currentPosition
  int nextIndex = routeIdx + 1;
  if (nextIndex >= routeLen) return -1;
  return route[nextIndex];
}

// Call this when your sensors detect you arrived at some node `arrivedNode`.
// It advances the route pointer.
void advanceWaypointIfArrived(int arrivedNode) {
  if (routeLen == 0) return;

  // If we arrived at the next waypoint, advance
  int nextWp = getNextWaypoint();
  if (nextWp != -1 && arrivedNode == nextWp) {
    routeIdx++;
    currentPosition = arrivedNode;

    Serial.print("Arrived at waypoint "); Serial.println(arrivedNode);

    if (routeIdx == routeLen - 1) {
      Serial.println("Reached goal!");
    }
  }
}

void printDestinations() {
  Serial.print("Destinations: ");
  for (int i = 0; i < destCount; i++) {
    Serial.print(destinations[i]);
    if (i < destCount - 1) Serial.print(", ");
  }
  Serial.println();
}
bool print = false;
void loop() {
  // nothing
  while (!finished){
    delay(200);
    runClientLoop();
    print = true;}
  if (print){
    printDestinations();
    print = false;}

}
