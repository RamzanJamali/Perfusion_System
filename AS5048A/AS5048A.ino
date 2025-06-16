#include <SPI.h>
#include <AS5048A.h>
const uint8_t CS = 10;
AS5048A enc(CS);

uint16_t prevAngle;
unsigned long prevTime;
const unsigned long interval = 10000; // 10 ms

void setup() {
  Serial.begin(115200);
  pinMode(CS, OUTPUT); digitalWrite(CS, HIGH);
  SPI.begin(); SPI.setDataMode(SPI_MODE1); SPI.setClockDivider(SPI_CLOCK_DIV16);
  prevAngle = enc.getRawRotation();
  prevTime = micros();
}

void loop() {
  if (micros() - prevTime >= interval) {
    uint16_t currAngle = enc.getRawRotation();
    unsigned long currTime = micros();

    int32_t delta = currAngle - prevAngle;
    if (delta > 8192) delta -= 16384;
    else if (delta < -8192) delta += 16384;

    float deltaTurns = delta / 16384.0;
    float deltaSeconds = (currTime - prevTime) * 1e-6;
    float rpm = deltaTurns * 60.0 / deltaSeconds;

    Serial.print("RPM: ");
    Serial.println(rpm, 2);

    prevAngle = currAngle;
    prevTime = currTime;
  }
}
