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
  return destination;
}

void sendTelemetry() {
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

// --- In Wifi.ino ---

// 1. The background task function
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

// 2. The function that launches the task
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

// 3. Keep your sendTelemetry() and resetServerMap() functions here 
// as they were, but they will now be called by Core 0.