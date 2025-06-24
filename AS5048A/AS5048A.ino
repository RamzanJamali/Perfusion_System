#include "AS5048A.h"

const uint8_t CS_PIN = 10;  // connect AS5048A CS to D10 //  
  /*For SPI, you'll connect the encoder's MISO, MOSI, SCK, and CS pins to the corresponding SPI pins 
  on your Arduino (typically D12, D11, D13, and D10 on an Uno/Nano, respectively). */
//SPISettings as5048aSPI(1000000, MSBFIRST, SPI_MODE1);

double rpm;

void setup() {
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI_begin();
}


void loop() {

  rpm = speed_in_rpm(CS_PIN);
  Serial.println(rpm, 9);
  delay(50);
}
