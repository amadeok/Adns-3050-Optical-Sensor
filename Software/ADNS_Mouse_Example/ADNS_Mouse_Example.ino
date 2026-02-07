#include "ADNS3050.h"
#include <SPI.h>
#include <Mouse.h>

byte x = 0;
byte y = 0;

void setup() {
    Serial.begin(115200);
  delay(1200);   
  Serial.println("Init");  
  startup();

  // Optional: read some status registers after init
  Serial.println(F("Initial status check:"));
  Read(MOTION_ST);
  Read(SQUAL);
  Read(SHUT_HI);
  Read(SHUT_LO);
  Serial.println();

  // Serial.println(F("Reading MOTION_CTRL for verification..."));
  // readCount = Read(MOTION_CTRL);
}

void loop() {

  // Serial.println("\n");
  // Write(MOTION_CTRL, readCount);

  // Serial.println(F("Reading MOTION_CTRL for verification..."));
  // byte motion_ctrl = Read(MOTION_CTRL);



   delay(10);
  // Simple demo: read motion every 100 ms
  // static unsigned long last = 0;
  // if (millis() - last > 100) {
  //   last = millis();
     getXY();
  // }
}