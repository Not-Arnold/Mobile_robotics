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



void getTurn(int pre, int cur, int next) {
  if (next == destinations[destCount - 2]) uturn = true;

    if (pre == 4 && cur == 0 && next == 1) {nextturn = 1; direction = "LEFT"; return;}
    if (pre == 4 && cur == 0 && next == 2) {nextturn = 1; direction = "STRAIGHT"; return;}

    if (pre == 3 && cur == 2 && next == 1) {nextturn = 1; direction = "RIGHT"; return;}
    if (pre == 3 && cur == 2 && next == 0) {nextturn = 1; direction = "STRAIGHT"; return;}

    if ((pre == 3 || pre == 4) && cur == 1 && next == 2) {nextturn = 1; direction = "LEFT"; return;}
    if ((pre == 3 || pre == 4) && cur == 1 && next == 0) {nextturn = 1; direction = "RIGHT"; return;}


    if (pre == 2 && cur == 3 && next == 1) {nextturn = 1; direction = "LEFT"; return;}
    if (pre == 2 && cur == 3 && next == 4) {nextturn = 1; direction = "STRAIGHT"; return;}

    if (pre == 0 && cur == 4 && next == 1) {nextturn = 1; direction = "RIGHT"; return;}
    if (pre == 0 && cur == 4 && next == 3) {nextturn = 1; direction = "STRAIGHT"; return;}

    if ((pre == 2 || pre == 0) && cur == 1 && next == 4) {nextturn = 1; direction = "LEFT"; return;}
    if ((pre == 2 || pre == 0) && cur == 1 && next == 3) {nextturn = 1; direction = "RIGHT"; return;}


   // new 24 if
    if (pre == 0 && cur == 1 && next == 2) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 0 && cur == 1 && next == 0) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 0 && cur == 2 && next == 1) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 0 && cur == 2 && next == 0) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;}

    if (pre == 1 && cur == 0 && next == 1) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 1 && cur == 0 && next == 2) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;}
    if (pre == 1 && cur == 2 && next == 1) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 1 && cur == 2 && next == 0) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;}    

    if (pre == 2 && cur == 1 && next == 2) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 2 && cur == 1 && next == 0) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 2 && cur == 0 && next == 1) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 2 && cur == 0 && next == 2) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;}    



    if (pre == 4 && cur == 1 && next == 4) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 4 && cur == 1 && next == 3) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 4 && cur == 3 && next == 1) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 4 && cur == 3 && next == 4) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;} 

    if (pre == 1 && cur == 4 && next == 1) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 1 && cur == 4 && next == 3) {nextturn = 1; uturn = true; direction = "STAIGHT"; return;}
    if (pre == 1 && cur == 3 && next == 1) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 1 && cur == 3 && next == 4) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;}       

    if (pre == 3 && cur == 1 && next == 3) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 3 && cur == 1 && next == 4) {nextturn = 1; uturn = true; direction = "LEFT"; return;}
    if (pre == 3 && cur == 4 && next == 1) {nextturn = 1; uturn = true; direction = "RIGHT"; return;}
    if (pre == 3 && cur == 4 && next == 4) {nextturn = 1; uturn = true; direction = "STRAIGHT"; return;} 

  nextturn = 0; 
  return "STRAIGHT";
}


void nagvigating(){
  if(millis() - lasttalktoserver > 300){

      if (nextturn == 0){
        curNode = currentPosition; 
        runClientLoop();
        nextNode = currentPosition;
        preNode = destinations[destCount - 2];
        getTurn(preNode, curNode, nextNode);}

      else {nextturn++;}

      if (uturn):{
        turning();
        uturn = false;
      }
      
      if (nextturn == 2){
        if (direction == "LEFT") {
            turningL();
            nextturn = 0;}

        else if (direction == "RIGHT"){
            turningR();
            nextturn = 0;}
        
        else if (direction == "STRAIGHT") {
          driveMotors(200, 200);
          delay(300);
          nextturn = 0;}}
      
      else if (direction == "UTURN"){
            turning();
      }
            
      else{
      driveMotors(200, 200);
      delay(300);}
      
      lasttalktoserver = millis();
}}


void turningL() {
  int turnSpeed = 200;  // A manageable speed for rotating
  int threshold = 500;

  digitalWrite(motor1Phase, HIGH);
  analogWrite(motor1PWM, turnSpeed);
  digitalWrite(motor2Phase, HIGH);
  analogWrite(motor2PWM, turnSpeed);

  delay(250);

  while (analogRead(AnalogPin[2]) > threshold) {}

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

  while (analogRead(AnalogPin[2]) > threshold) {}

  stopMotors();
  delay(200);
}


