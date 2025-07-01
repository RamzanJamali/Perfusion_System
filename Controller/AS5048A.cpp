#include "AS5048A.h"


SPISettings as5048aSPI(1000000, MSBFIRST, SPI_MODE1);


void SPI_begin(){
  SPI.begin();
}


uint16_t read_raw_angle(const uint8_t CS_PIN) {
  uint16_t dummy, raw;

  // 1) prime: pull out the OLD register (ignore it)
  SPI.beginTransaction(as5048aSPI);
  digitalWrite(CS_PIN, LOW);
    dummy = SPI.transfer16(0xFFFF);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  delayMicroseconds(1); // let the chip update its output register

  // 2) real read: this is the fresh angle + error bit
  SPI.beginTransaction(as5048aSPI);
  digitalWrite(CS_PIN, LOW);
    raw = SPI.transfer16(0xFFFF);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  // now *this* is where you check for an error
  if (raw & 0x4000) {
    Serial.println("⚠️ AS5048A magnetic error");
  }

  // finally mask off error + parity, return only the 14-bit angle
  return raw & 0x3FFF;
}


double raw_to_radians(uint16_t raw) {
  // raw = 0…16383 maps to 0…2π
  return (double)raw * (9.0f * PI / 16383.0f);
}


double speed_in_rpm(const uint8_t CS_PIN) {
  static double prevAngle = 0.0;
  static unsigned long prevMicros = 0;
  static double rpm = 0.0;
Work here
  // 1) read current angle + timestamp
  uint16_t raw = read_raw_angle(CS_PIN);
  float angle = raw_to_radians(raw);
  unsigned long now = micros();

  if (prevMicros != 0) {
    // 2) compute Δangle, correcting for wrap-around
    float delta_angle = angle - prevAngle;
    if (delta_angle >  PI) delta_angle -= 2 * PI;
    if (delta_angle < -PI) delta_angle += 2 * PI;

    // 3) compute Δt in seconds
    float dt = (now - prevMicros) * 1e-6f;

    // 4) angular speed [rad/s]
    float omega = fabsf(delta_angle) / dt;

    // 5) convert to RPM: ω (rad/s) * (60 / 2π)
    rpm = omega * (60.0f / (2.0f * PI));

    Serial.print("RPM: ");
    Serial.println(rpm, 9);
  }

  // save for next iteration
  prevAngle  = angle;
  prevMicros = now;

  return rpm;
}