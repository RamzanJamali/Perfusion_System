#include <SPI.h>

const int CS_PIN = 10;  // adjust as needed
#define READ_ANGLE 0x4000
#define READ_AGC   0x3FFD

uint16_t spiTransfer16(uint16_t cmd) {
  uint16_t response;
  digitalWrite(CS_PIN, LOW);
  response = SPI.transfer16(cmd);
  digitalWrite(CS_PIN, HIGH);
  return response;
}

uint16_t readRegister(uint16_t regAddr) {
  uint16_t cmd = 0x4000 | (regAddr & 0x3FFF);  // read command
  // send read + dummy to get result from second frame
  spiTransfer16(cmd);
  return spiTransfer16(READ_ANGLE);  // second frame returns data
}

void setup(){
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
}

void loop(){
  uint16_t rawAngle  = readRegister(0x3FFF) & 0x3FFF;
  uint16_t agc       = readRegister(0x3FFD) >> 8; // upper 8 bits AGC
  Serial.print("Angle: "); Serial.print(rawAngle);
  Serial.print("   AGC: ");   Serial.println(agc);
  delay(200);
}
