#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define DEBUG

//Pins
#define SS        18
#define RST       14
#define DI0       26
#define gps_RX    22
#define gps_TX    23
#define GPS_baud  9600


// Tiny GPS object
TinyGPSPlus gps;

// global variables
int n = 1;
String str;
char gps_lat[15], gps_lng[15];
float last_lat, last_lng;

HardwareSerial gps_serial(1);

void setup() {
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
}




void SaveCoordinates() {
  dtostrf (gps.location.lat(), 2, 8, gps_lat);
  dtostrf (gps.location.lng(), 2, 8, gps_lng);
}





void loop() {
  while (gps_serial.available() > 0) {
    if (gps.encode(gps_serial.read())) {
      if (gps.location.isValid() && ((float)last_lat != (float)gps.location.lat() || (float)last_lng != (float)gps.location.lng())) {
        // Este IF ainda vai ser mudado. Neste momento envia a localização quando ela é diferente, mas vai ser em função de um timer i.e. de 10 em 10 min
        SaveCoordinates();
        str = "Packet #" + (String)n + ":;" + "Lat:" + (String)gps_lat + ";Lng:" + (String)gps_lng + ";";
        // send packet
        if (LoRa.beginPacket()) {
          LoRa.print(str);
          LoRa.endPacket();
          n++;
          last_lat = gps.location.lat();
          last_lng = gps.location.lng();
          
        #ifdef DEBUG
          Serial.println("Sent: " + str);
        #endif
        }
      }
      else {
        #ifdef DEBUG
          Serial.println("nothing");
        #endif
      }
    }
  }
  delay(1000);
}
