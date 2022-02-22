#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define DEBUG
#define EEPROM_SIZE 1


#define SS        18
#define RST       14
#define DI0       26
#define gps_RX    22
#define gps_TX    23
#define GPS_baud  9600

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

#define convert_to_microseconds 1000000
#define rtc_timer_in_seconds 10


// Tiny GPS object
TinyGPSPlus gps;

// global variables
int n = 1;
String str;
char gps_lat[15], gps_lng[15];
int bootCount = 0;
bool sent_flag = false;

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

HardwareSerial gps_serial(1);



void setupWifi(){
  // Connect to Wi-Fi network with SSID and password
  #ifdef DEBUG
    Serial.print("Setting AP (Access Point)…");
  #endif
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(SSID, PASS, 1, 0, 1);

  IPAddress IP = WiFi.softAPIP();
  #ifdef DEBUG
    Serial.print("AP IP address: ");
    Serial.println(IP);
  #endif
}

void setupWebServer(){
  #ifdef DEBUG
    Serial.print("Start Server setup");
  #endif
  server.on("/get", HTTP_GET, handleGet);
  server.on("/info", sendModuleInfo);
  server.on("/start", startMission);
  server.begin();
  #ifdef DEBUG
    Serial.println("... started");
  #endif
}

void handleGet(){
  String data = "{\"status\":\"unsuccessful\"}";
  String op;

  // Update Operating Mode1 and operation
  if (server.hasArg("mode1")) {
    op = server.arg("mode1");
    opMode1 = op.toInt();
    #ifdef DEBUG
      Serial.println("Mode1: " + op);
    #endif
    data = "{\"status\":\"successful\"}";
    if (server.hasArg("op")) {
      operation = server.arg("op");
      #ifdef DEBUG
        Serial.println("Operation: " + operation);
      #endif
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
    #ifdef DEBUG
      Serial.println("Mode2: " + op);
    #endif
    data = "{\"status\":\"successful\"}";
  }
  else{
    opMode2 = NO_MODE;
  }

  // Update the timerValue
  if(server.hasArg("timer")){
    String timer = server.arg("timer");
    timerValue = timer.toInt();
    #ifdef DEBUG
      Serial.println("Timer Value: " + timerValue);
    #endif
  } 
  else{
    timerValue = 0;
  }

  // Update the Depth reference
  if(server.hasArg("depth")){
    String depthS = server.arg("depth");
    depthRef = depthS.toInt();
    #ifdef DEBUG
      Serial.println("Depth Reference: " + String(depthRef));
    #endif
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
  #ifdef DEBUG
    Serial.println("data sent: " + data);
  #endif
  server.send(200, "application/json", data);
}

void startMission(){
  
  String data = "{\"status\":\"successful\"}";
  #ifdef DEBUG
    Serial.println("Starting Mission");
  #endif
  
  server.send(200, "application/json", data);

  hibernate();
}




void openValve(){
  // open the servo valve
  onMission = false;
  valveMotor.attach(SERVO_PIN);
  valveMotor.write(COUNTER_CLOCKWISE_ROTATION);
  digitalWrite(33, HIGH);
  #ifdef DEBUG
    Serial.println("EJECT!!!");
  #endif
  delay(1500);  
  valveMotor.detach();
  delay(10000);
  valveMotor.attach(SERVO_PIN);
  valveMotor.write(CLOCKWISE_ROTATION);
  delay(1500);
  valveMotor.detach();
  digitalWrite(33, LOW);
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
  EEPROM.begin(EEPROM_SIZE);
  bootCount = EEPROM.read(0);

  if (bootCount == 1){
    SPI.begin(5, 19, 27, 18);
    LoRa.setPins(SS, RST, DI0);
    gps_serial.begin(9600, SERIAL_8N1, gps_RX, gps_TX);
  
    #ifdef DEBUG
      Serial.begin(9600);
      delay(1000);
      Serial.println("LoRa Transmitter");
    #endif
  
    if (!LoRa.begin(433E6)) {
      #ifdef DEBUG
          Serial.println("Starting LoRa failed!");
      #endif
      while (1);
    }
    setupServo();
    openValve();
  }

  else {
    #ifdef DEBUG
      Serial.begin(9600);
    #endif
    // Setup WiFi
    setupWifi();
    delay(100);
  
    // setup WebServer
    setupWebServer();
    delay(100);
  }
  delay(500);
}



void hibernate(){
  EEPROM.write(0, 1);
  EEPROM.commit();
  esp_sleep_enable_timer_wakeup(timerValue * convert_to_microseconds);
  esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
}



void SaveCoordinates() {
  dtostrf (gps.location.lat(), 2, 8, gps_lat);
  dtostrf (gps.location.lng(), 2, 8, gps_lng);
}





void loop() {
  if (bootCount == 0){
    server.handleClient();
  }
  else {
    EEPROM.write(0, 0);
    EEPROM.commit();
    while (gps_serial.available() > 0) {
      if (gps.encode(gps_serial.read())) {
        if (gps.location.isValid()) {
          SaveCoordinates();
          str = "Packet #" + (String)n + ":;" + "Lat:" + (String)gps_lat + ";Lng:" + (String)gps_lng + ";";
          // send packet
          if (LoRa.beginPacket()) {
            LoRa.print(str);
            LoRa.endPacket();
            n++;     
          #ifdef DEBUG
            Serial.println("Sent: " + str);
          #endif
          }
        }
        else {
          #ifdef DEBUG
            Serial.println("location not available");
          #endif
        }
        delay(5000); // try to send coordinates every x seconds
      }
    }
  }
}
