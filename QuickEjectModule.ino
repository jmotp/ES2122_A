#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

#define NO_MODE           0
#define TIMER_MODE        1
#define DEPTH_MODE        2
#define BATTERY_MODE      3


#define SSID    "Quick Eject Module"
#define PASS    "equipa#A"
#define SERVO_PIN  13
#define MIN_BATTERY 10
#define CLOCKWISE_ROTATION 180
#define COUNTER_CLOCKWISE_ROTATION 0
#define ALMOST_STOPPED 100


// Setup WebServer ESP32
WebServer server(80); 
Servo valveMotor;
 
int opMode1 = NO_MODE;
int opMode2 = NO_MODE;
String operation = "";
boolean onMission = false;

long int timerValue = 0;
long int timeLeft = 0;


long int depthRef = 0;
long int depth = 0;

long int batteryLevel = 100;


//timer variables
volatile int interrupts;
int seconds;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// runs every second on interrupt
void IRAM_ATTR onTime() {
  portENTER_CRITICAL_ISR(&timerMux);
  interrupts++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setupTimer(){
  // Configure Prescaler to 80, as our timer runs @ 80Mhz
  // Giving an output of 80,000,000 / 80 = 1,000,000 ticks / second
  timer = timerBegin(0, 80, true);                
  timerAttachInterrupt(timer, &onTime, true);    
  // Fire Interrupt every 1m ticks, so 1s
  timerAlarmWrite(timer, 1000000, true);      
  timerAlarmEnable(timer);
}

void setupWifi(){
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(SSID, PASS, 1, 0, 1);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void setupWebServer(){
  Serial.print("Start Server setup");
  server.on("/get", HTTP_GET, handleGet);
  server.on("/info", sendModuleInfo);
  server.on("/start", startMission);
  server.begin();
  Serial.println("... started");
}

void handleGet(){
  String data = "{\"status\":\"unsuccessful\"}";
  String op;

  // Update Operating Mode1 and operation
  if (server.hasArg("mode1")) {
    op = server.arg("mode1");
    opMode1 = op.toInt();
    Serial.println("Mode1: " + op);
    data = "{\"status\":\"successful\"}";
    if (server.hasArg("op")) {
      operation = server.arg("op");
      Serial.println("Operation: " + operation);
    }
    else{
      operation = "";
    }
  }
  else{
    opMode1 = NO_MODE;
    operation = "";
  }

  // Update Operation Mode2
  if (server.hasArg("mode2")) {
    op = server.arg("mode2");
    opMode2 = op.toInt();
    Serial.println("Mode2: " + op);
    data = "{\"status\":\"successful\"}";
  }
  else{
    opMode2 = NO_MODE;
  }

  // Update the timerValue
  if(server.hasArg("timer")){
    String timer = server.arg("timer");
    timerValue = timer.toInt();
    Serial.println("Timer Value: " + timerValue);
  } 
  else{
    timerValue = 0;
  }

  // Update the Depth reference
  if(server.hasArg("depth")){
    String depthS = server.arg("depth");
    depthRef = depthS.toInt();
    Serial.println("Depth Reference: " + String(depthRef));
  } 
  else{
    depthRef = 0;
  }
  server.send(200, "application/json", data);
}

void sendModuleInfo(){
  int operat;
  if (operation == "OR"){
    operat = 1;
  }
  else if(operation == "AND"){
    operat = 2;
  }
  else{
    operat = 0;
  }

  String data = "{\"status\":"+String(opMode1)+"-"+
                String(operat)+"-"+String(opMode2)+
                "-"+String(timerValue)+"}";

  Serial.println("data sent: " + data);
  server.send(200, "application/json", data);
}

void startMission(){
  
  String data = "{\"status\":\"successful\"}";
  Serial.println("Starting Mission");
  if(opMode1 == TIMER_MODE || opMode2 == TIMER_MODE){
    seconds = 0;
    Serial.println("Starting Timer");
  }
  onMission = true;
  server.send(200, "application/json", data);
}

boolean getOpModeStatus(int operatMode){
  switch(operatMode){
    case TIMER_MODE:  if(seconds >= timerValue){    // Check if timer has reached the pretended value
                          return true;
                      }
                      break;
    case DEPTH_MODE:  if(depth >= depthRef){       // Check if depth has reached the pretended value
                          return true;
                      }
                      break;
    case BATTERY_MODE:  if(batteryLevel <= MIN_BATTERY){       // Check if battery has reached the minimum value to eject
                          return true;
                      }
                      break;
    default:  break;                    
  }

  return false;
}


void checkTime(){
  // Start Timer for the Mission and Enter Loop
  // to be replaced with interrupt
  timeLeft = timerValue - seconds;
  Serial.print("Seconds left: ");
  Serial.println(timeLeft);
}

void openValve(){
  // open the servo valve
  onMission = false;
  valveMotor.attach(SERVO_PIN);
  valveMotor.write(COUNTER_CLOCKWISE_ROTATION);
  digitalWrite(33, HIGH);
  Serial.println("EJECT!!!");
  delay(1500);  
  valveMotor.detach();
  delay(10000);
  valveMotor.attach(SERVO_PIN);
  valveMotor.write(CLOCKWISE_ROTATION);
  delay(1500);
  valveMotor.detach();
  digitalWrite(33, LOW);
}

void checkForDeployment(){
  if(opMode2 == NO_MODE && getOpModeStatus(opMode1)){
    openValve();
  }
  else if (opMode1 == NO_MODE && getOpModeStatus(opMode2)){
    openValve();
  } 
  else{
    if(operation == "AND"){
      if(getOpModeStatus(opMode1) & getOpModeStatus(opMode2)){
        openValve();
      }
    }
    else if(operation == "OR"){
      if(getOpModeStatus(opMode1) | getOpModeStatus(opMode2)){
        openValve();
      }
    }
  }    
}

void setupServo(){
  
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  valveMotor.setPeriodHertz(50);// Standard 50hz servo
  
  //valveMotor.attach(SERVO_PIN);
}

void setup() {
  // Configure Serial Port
  Serial.begin(115200);

  // Setup WiFi
  setupWifi();
  delay(100);

  // setup WebServer
  setupWebServer();
  delay(100);

  // setup timer interrupt
  setupTimer();

  // Setup Servo
  setupServo();

  //LED for testing
  pinMode(33, OUTPUT);

  Serial.println("Setup Done");  
  Serial.println("Mode1: " + String(opMode1) + "\t   Operator: " + operation + "\t   Mode2: " + String(opMode2) + "\t   timerValue: " + String(timerValue));

}

void loop() {
  if (interrupts > 0) {
    portENTER_CRITICAL(&timerMux);
    interrupts--;
    portEXIT_CRITICAL(&timerMux);
    seconds++;
    //Serial.println(String(seconds));
    if(onMission){
      checkForDeployment();
      checkTime();
    }
  }
  else{
    server.handleClient();
  }

}
