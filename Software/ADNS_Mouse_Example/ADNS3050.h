#include "USBAPI.h"
#include <SPI.h>

 // SPI and misc pins for the ADNS
 #define PIN_SCLK   SCK
 #define PIN_MISO   MISO
 #define PIN_MOSI   MOSI
 #define PIN_NCS    10
 #define PIN_MOTION 5


// Define all the registers
 #define PROD_ID                          0x00
 #define REV_ID                           0x01
 #define MOTION_ST                        0x02
 #define DELTA_X                          0x03
 #define DELTA_Y                          0x04
 #define SQUAL                            0x05
 #define SHUT_HI                          0x06
 #define SHUT_LO                          0x07
 #define PIX_MAX                          0x08
 #define PIX_ACCUM                        0x09
 #define PIX_MIN                          0x0a
 #define PIX_GRAB                         0x0b
 #define MOUSE_CTRL                       0x0d
 #define RUN_DOWNSHIFT                    0x0e
 #define REST1_PERIOD                     0x0f
 #define REST1_DOWNSHIFT                  0x10
 #define REST2_PERIOD                     0x11
 #define REST2_DOWNSHIFT                  0x12
 #define REST3_PERIOD                     0x13
 #define PREFLASH_RUN                     0x14
 #define PREFLASH_RUN_DARK                0x18
 #define MOTION_EXT                       0x1b
 #define SHUT_THR                         0x1c
 #define SQUAL_THRESHOLD                  0x1d
 #define NAV_CTRL2                        0x22
 #define MISC_SETTINGS                    0x25
 #define RESOLUTION                       0x33
 #define LED_PRECHARGE                    0x34
 #define FRAME_IDLE                       0x35
 #define RESET                            0x3a
 #define INV_REV_ID                       0x3f
 #define LED_CTRL                         0x40
 #define MOTION_CTRL                      0x41
 #define AUTO_LED_CONTROL                 0x43
 #define REST_MODE_CONFIG                 0x45


void com_start() {
  Serial.println(F("[CS] Starting communication burst"));
  digitalWrite(PIN_NCS, HIGH);
  delay(20);
  digitalWrite(PIN_NCS, LOW);
  Serial.println(F("[CS] NCS → LOW (communication started)"));
}

// byte Read(byte reg_addr){
//   digitalWrite(PIN_NCS, LOW);//begin communication
//   // send address of the register, with MSBit = 0 to say it's reading
//   SPI.transfer(reg_addr & 0x7f );
//   delayMicroseconds(100);
//   // read data
//   byte data = SPI.transfer(0);
//   delayMicroseconds(30);
//   digitalWrite(PIN_NCS, HIGH);//end communication
//   delayMicroseconds(30);

//   return data;
// }
int readCount = 0;
byte Read(byte reg_addr) {
  Serial.print(F("[RD] Reading register 0x"));
  if (reg_addr < 0x10) Serial.print("0");
  Serial.println(reg_addr, HEX);

  digitalWrite(PIN_NCS, LOW);
  delayMicroseconds(10);

  // Send address (MSB=0 for read)
  SPI.transfer(reg_addr & 0x7F);
  delayMicroseconds(100);

  // Read data
  byte data = SPI.transfer(0);
  delayMicroseconds(30);

  digitalWrite(PIN_NCS, HIGH);
  delayMicroseconds(30);

  Serial.print(F("[RD] Value = 0x"));
  if (data < 0x10) Serial.print("0");
  Serial.print(data, HEX);
  Serial.print(" (");
  Serial.print(data, DEC);
  Serial.print(")");
  Serial.print("(c ");
  Serial.print(readCount++, DEC);
  Serial.println(")");

  return data;
}

void Write(byte reg_addr, byte data) {
  Serial.print(F("[WR] Writing register 0x"));
  if (reg_addr < 0x10) Serial.print("0");
  Serial.print(reg_addr, HEX);
  Serial.print(" ← 0x");
  if (data < 0x10) Serial.print("0");
  Serial.println(data, HEX);

  digitalWrite(PIN_NCS, LOW);
  delayMicroseconds(10);

  SPI.transfer(reg_addr | 0x80);   // MSB=1 → write
  SPI.transfer(data);
  delayMicroseconds(50);

  digitalWrite(PIN_NCS, HIGH);
  delayMicroseconds(50);
}

void startup() {
                // Give serial monitor time to connect
  Serial.println(F("\n=== ADNS Sensor Startup ==="));
  Serial.println(F("Build date: " __DATE__ " " __TIME__));

  pinMode(PIN_MISO, INPUT);
  pinMode(PIN_NCS, OUTPUT);
  digitalWrite(PIN_NCS, HIGH);    // idle high

  Serial.println(F("Initializing SPI..."));
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // 1 MHz
  Serial.println(F("SPI configured: Mode 3, 1 MHz, MSBFIRST"));

  delay(50);

  Serial.println(F("Performing soft reset..."));
  com_start();
  Write(RESET, 0x5A);
  Serial.println(F("Reset command sent (0x5A)"));
  delay(150);

  Serial.println(F("Configuring mouse control..."));
  Write(MOUSE_CTRL, 0x20);
  delay(20);

  // Serial.println(F("Clearing motion control register..."));
  // Write(MOTION_CTRL, 0x00);
  // delay(100);

  // Optional: read product ID to verify communication
  Serial.println(F("Reading Product ID for verification..."));
  byte prod_id = Read(PROD_ID);
  Serial.print(F("→ Product ID = 0x"));
  Serial.println(prod_id, HEX);

  byte rev_id = Read(REV_ID);
  Serial.print(F("→ Revision ID = 0x"));
  Serial.println(rev_id, HEX);

  Serial.println(F("=== Startup sequence finished ===\n"));
}

int convTwosComp(int b) {
  if (b & 0x80) {
    b = -1 * ((b ^ 0xFF) + 1);
  }
  return b;
}

void getXY() {
  Serial.println(F("--- Reading motion deltas ---"));

  byte mot = Read(MOTION_ST);
  Serial.print(F("MOTION register: 0x"));
  Serial.print(mot, HEX);

  if (mot & 0x80) {
    Serial.println(F(" (Motion detected)"));
  } else {
    Serial.println(F(" (No motion)"));
  }

  byte x = Read(DELTA_X);
  byte y = Read(DELTA_Y);

  int dx = convTwosComp(x);
  int dy = convTwosComp(y);

  Serial.print(F("Δx = "));
  Serial.print(dx);
  Serial.print(F("   (raw byte = 0x"));
  if (x < 0x10) Serial.print("0");
  Serial.print(x, HEX);
  Serial.println(")");

  Serial.print(F("Δy = "));
  Serial.print(dy);
  Serial.print(F("   (raw byte = 0x"));
  if (y < 0x10) Serial.print("0");
  Serial.print(y, HEX);
  Serial.println(")\n");
}

int getX() {
  byte x = Read(DELTA_X);
  return convTwosComp(x);
}

int getY() {
  byte y = Read(DELTA_Y);
  return convTwosComp(y);
}

// ────────────────────────────────────────────────
//                  Typical setup() / loop()
// ────────────────────────────────────────────────



