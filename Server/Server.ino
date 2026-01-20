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
#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#endif

/* ==================================================
   WiFi credentials
   ================================================== */
const char* ssid     = "Alex's A22";
const char* password = "asdfghjkl";

/* ==================================================
   Team and server configuration
   ================================================== */
const char* TEAM_ID = "asun2881";
const char* SERVER_BASE = "http://3.250.38.184:8000";

/* ==================================================
   notifyArrival()
   ================================================== */
String notifyArrival(int position) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return "";
  }

  String url = String(SERVER_BASE) + "/api/arrived/" + TEAM_ID;

#ifdef ESP32
  HTTPClient http;
  http.begin(url);
#else
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
#endif

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
    Serial.print(".");
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

  // The fixed test sequence of ARRIVAL positions you requested.
  // "const" keeps it read-only; "static" gives it persistent storage.
  static const int arrivalSequence[] = {0,1,2,3,4,3,2,0,4,5};
  static const size_t sequenceLength = sizeof(arrivalSequence) / sizeof(arrivalSequence[0]);

  // Index of which step we are on in the sequence.
  // "static" means it remembers its value between loop() calls.
  static size_t idx = 0;

  // Rate limit so you don't hammer the server (adjust as needed).
  // 1000–3000 ms is usually reasonable for testing.
  static unsigned long lastSendMs = 0;
  const unsigned long SEND_INTERVAL_MS = 2000;

  // If enough time has passed, send the next simulated arrival.
  unsigned long now = millis();
  if (now - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = now;

    int arrivedPosition = arrivalSequence[idx];

    Serial.println();
    Serial.println("========================================");
    Serial.print("SIM: Arrived at position ");
    Serial.println(arrivedPosition);

    // Notify server and read destination (plain text).
    String destStr = notifyArrival(arrivedPosition);

    // Also print as integer if it looks numeric.
    if (destStr.length() > 0) {
      int dest = destStr.toInt();
      Serial.print("Parsed destination int: ");
      Serial.println(dest);
    } else {
      Serial.println("No destination received (empty response).");
    }

    // Advance to next step in the sequence, wrapping around.
    idx = (idx + 1) % sequenceLength;
  }

  // loop() continues running; we only POST when the interval elapses.
}