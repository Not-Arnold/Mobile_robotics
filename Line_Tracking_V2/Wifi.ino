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

  // Notify server of arrival at CURRENT position
  String response = notifyArrival(currentPosition);

  if (response.length() == 0) {
    return;
  }

  if (response == "Finished") {
    finished = true;
    return;
  }

  int nextNode = response.toInt();
  currentPosition = nextNode;
  destinations[destCount++] = nextNode;
}