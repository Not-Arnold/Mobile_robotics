// ============================================================
// Wifi.ino — Server Communication & Telemetry
//
// Handles:
// 1. Notifying the competition server when the robot arrives
//    at a node.
// 2. Sending live telemetry to the dashboard server.
// 3. Resetting the dashboard map when the robot reconnects.
// 4. Running telemetry in a separate FreeRTOS task on Core 0.
// ============================================================


// -------------------------------------------------------
// notifyArrival — Tell the competition server which node
// the robot has just reached.
//
// Sends a POST request to:
//   SERVER_BASE/api/arrived/TEAM_ID
//
// Body format:
//   position=<node_id>
//
// Returns:
//   - The server response as a String (usually the next target)
//   - "" if Wi-Fi is disconnected or the request fails
// -------------------------------------------------------
String notifyArrival(int position) {
  // Abort immediately if Wi-Fi is not connected
  if (WiFi.status() != WL_CONNECTED) {
    return "";
  }
  
  // Build the arrival endpoint URL
  String url = String(SERVER_BASE) + "/api/arrived/" + TEAM_ID;

  HTTPClient http;
  http.begin(url);

  // Server expects form-encoded data
  String body = "position=" + String(position);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send POST request
  int httpCode = http.POST(body);

  String destination = "";

  // If request succeeded, read response body
  if (httpCode > 0) {
    destination = http.getString();
    destination.trim();
  }
  http.end();
  return destination;
}

// -------------------------------------------------------
// sendTelemetry — Push live robot data to the dashboard
//
// Sends a JSON payload to:
//   BASE_URL/telemetry/ROBOT_ID
//
// Includes:
//   - current node
//   - next target node
//   - left/right motor PWM
//   - robot state string
//   - current route string
//   - optional obstacle edge if present
//
// Uses HTTPS with WiFiClientSecure.
// setInsecure() disables certificate validation, which is
// convenient for testing/tunnels but not ideal for production.
// -------------------------------------------------------
void sendTelemetry() {
  // Do nothing if Wi-Fi is down
  if (WiFi.status() != WL_CONNECTED) return;
  
  WiFiClientSecure Client;
  Client.setInsecure(); // Required for Cloudflare/HTTPS 
  HTTPClient http;
  
  // Use the current URL from config.h and append the endpoint 
  http.begin(Client, BASE_URL + "/telemetry/" + ROBOT_ID);
  //http.setTimeout(50); // Set a 50ms timeout - very important!
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Token", API_TOKEN);

  // Construct JSON - Ensure keys match what server.py expects 
  String json = "{";
  json += "\"curr\":" + String(currentPosition) + ",";
  json += "\"next\":" + String(nextnode) + ","; // Match lowercase 'n' from config.h 
  json += "\"pwmL\":" + String(currentLeftSpeed) + ",";
  json += "\"pwmR\":" + String(currentRightSpeed) + ",";
  json += "\"state\":\"" + stateStr + "\",";
  json += "\"route\":\"" + routeStr + "\"";

  // Only send obstacles if they are valid (initialized to -1) 
  if (obstacleStartNode != -1 && obstacleEndNode != -1) {
    json += ",\"obs_u\":" + String(obstacleStartNode);
    json += ",\"obs_v\":" + String(obstacleEndNode);
  }

  json += "}";

  int httpResponseCode = http.POST(json);
  
  // Debugging: check if the server is actually receiving it
  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error occurred: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
}

// -------------------------------------------------------
// resetServerMap — Clear the dashboard's remembered obstacles
// and route display for this robot.
//
// Sends a POST request to:
//   BASE_URL/reset/ROBOT_ID
//
// Called once when the telemetry task starts, so the dashboard
// begins in a clean state after reconnecting or rebooting.
// -------------------------------------------------------
void resetServerMap() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  WiFiClientSecure Client;
  Client.setInsecure(); 
  HTTPClient http;
  
  // Connect to the new reset endpoint
  http.begin(Client, BASE_URL + "/reset/" + ROBOT_ID);
  
  int httpResponseCode = http.POST(""); // Empty body is fine
  
  if (httpResponseCode > 0) {
    Serial.println("Server Map Reset Successful");
  } else {
    Serial.printf("Reset failed: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}

// -------------------------------------------------------
// telemetryTask — Background FreeRTOS task for dashboard updates
//
// Runs forever on Core 0.
// Behaviour:
//   1. Resets dashboard state once on startup
//   2. Sends telemetry roughly every 500 ms
//   3. Yields frequently so the system stays stable
//
// This keeps network traffic off the main control loop.
// -------------------------------------------------------
void telemetryTask(void * pvParameters) {
  Serial.print("Telemetry Task starting on core: ");
  Serial.println(xPortGetCoreID());

  // Call reset once upon connection
  resetServerMap();

  for(;;) {
    // Send updates every 500ms
    if (millis() - lastTelemetryTime > 500) {
      lastTelemetryTime = millis();
      sendTelemetry(); 
    }
    // Yield to the system (essential for Core 0 stability)
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// -------------------------------------------------------
// setupMultiCore — Launch the telemetry task on Core 0
//
// Task settings:
//   Name:       "Telemetry"
//   Stack size: 10000
//   Priority:   1
//   Core:       0
//
// Your main robot logic can remain on the other core / main loop.
// -------------------------------------------------------
void setupMultiCore() {
  xTaskCreatePinnedToCore(
    telemetryTask,    /* Task function */
    "Telemetry",      /* Name of task */
    10000,            /* Stack size */
    NULL,             /* Parameter */
    1,                /* Priority */
    NULL,             /* Task handle */
    0                 /* Pin to Core 0 */
  );
}
