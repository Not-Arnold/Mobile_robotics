/****************************************************
 *  WiFi Arrival Notification Client (ESP32 / ESP8266)
 *
 *  Purpose:
 *  - Connect to a WiFi network
 *  - Notify the EE303 Mobile Robotics Server that
 *    the robot has arrived at a specific position
 *  - Receive the next destination from the server
 *
 *  Server:
 *    http://3.250.38.184:8000
 *
 *  Endpoint:
 *    POST /api/arrived/TEAM-ID
 *
 *  Body format:
 *    application/x-www-form-urlencoded
 *    position=<number>
 *
 *  Response:
 *    Plain text (e.g. "5")
 ****************************************************/

/* ==================================================
   Platform-specific includes
   ================================================== */
  #include <WiFi.h>
  #include <HTTPClient.h>

/* ==================================================
   WiFi credentials
   ================================================== */
const char* ssid     = "IOT";
const char* password = "repacks43telangiectases";

/* ==================================================
   Team and server configuration
   ================================================== */
const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";


bool finished = false;
/* ==================================================
   notifyArrival()
   ================================================== */
String notifyArrival(int position) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return "";
  }

  String url = String(SERVER_BASE) + "/api/arrived/" + TEAM_ID;


  HTTPClient http;
  http.begin(url);

  String body = "position=" + String(position);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  Serial.print("POST URL: ");
  Serial.println(url);
  Serial.print("POST body: ");
  Serial.println(body);

  int httpCode = http.POST(body);

  String destination = "";

  if (httpCode > 0) {
    Serial.print("HTTP response code: ");
    Serial.println(httpCode);

    destination = http.getString();
    destination.trim();

    Serial.print("Server returned destination: ");
    Serial.println(destination);
  } else {
    Serial.print("HTTP POST failed: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
  return destination;
}

/* ==================================================
   setup()
   ================================================== */
void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("not yet");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("Robot IP address: ");
  Serial.println(WiFi.localIP());

  // Optional: one immediate test post at startup (you can keep or remove)
  String response = notifyArrival(0);
  if (response.length() > 0) {
    int nextDestination = response.toInt();
    Serial.print("Next destination (int): ");
    Serial.println(nextDestination);
  }
}

/* ==================================================
   loop()
   ==================================================
   Simulation goal:
   - Repeatedly "arrive" at: 0,1,2,3,2,3
   - After each simulated arrival:
       - POST position=<value> to the server
       - Print the returned destination
   - Repeat forever for testing
*/
void loop() {
  int currentPosition = 0;
  static unsigned long lastSendMs = 0;
  const unsigned long SEND_INTERVAL_MS = 2000;

  if (finished) return;

  if (millis() - lastSendMs < SEND_INTERVAL_MS) return;
  lastSendMs = millis();

  Serial.println();
  Serial.print("ARRIVED at node ");
  Serial.println(currentPosition);

  // Notify server of arrival at CURRENT position
  String response = notifyArrival(currentPosition);

  if (response.length() == 0) {
    Serial.println("Empty response – retry next cycle");
    return;
  }

  if (response == "Finished") {
    Serial.println("Mission complete");
    finished = true;
    return;
  }

  // Server gives NEXT node
  int nextNode = response.toInt();

  Serial.print("Server says go to node ");
  Serial.println(nextNode);

  currentPosition = nextNode;
}
