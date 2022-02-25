#include <U8x8lib.h>
#include <SPI.h>
#include <LoRa.h>



// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

//#define DEBUG

#define SS      18
#define RST     14
#define DI0     26


// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


int rssi;
String str;
char aux[64];

void setup() {
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  #ifdef DEBUG
    Serial.begin(9600);
    while (!Serial);
    delay(1000);
  #endif
  
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.drawString(0, 0, "LoRa Receiver");

  if (!LoRa.begin(433E6)) {
    u8x8.drawString(0, 0, "Starting LoRa failed!");
    while (1);
  }
}



void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    str = "Received:;";
    
    // read packet
    while (LoRa.available()) {
      str += (char)(LoRa.read());
    }

    // RSSI = Received Signal Strength Indicator
    rssi = LoRa.packetRssi(); 
    str = str + "RSSI: " + String(rssi) + ";";

    #ifdef DEBUG
      Serial.println(str);
    #endif
    
    int row = 2;
    int str_size = str.length();
    
    for (int i=0, j=0; i < str_size; i++){
      if (str[i] != ';'){
        aux[j] = str[i];
        j++;
        continue;
      }
   
      aux[j] = '\0';

      #ifdef DEBUG
        Serial.print("Screen Row #");
        Serial.print(row);
        Serial.print(": ");
        Serial.println(aux);
      #endif
      
      u8x8.drawString(0, row, aux);
      
      row++;
      j = 0;
    }  
  }
}
