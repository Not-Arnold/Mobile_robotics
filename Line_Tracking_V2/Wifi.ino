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



void runClientLoop() {
  if (finished) return; 

  String response = notifyArrival(currentPosition);
  if (response.length() == 0) return;

  if (response == "Finished") {
    finished = true; 
    return;
  }

  int targetNode = response.toInt(); 
  
  // Calculate path from current spot to the new goal
  findShortestPath(currentPosition, targetNode); 

  // IMPORTANT: Reset position and index for the new journey
  pathIndex = 0; 
  currentPosition = targetNode; 
  
  destCount++; 
  destinations[destCount] = targetNode; 
}