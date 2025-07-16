#include "AS5048A.h"
#include <SPI.h>

#define AS5048A_RESOLUTION       0.02197399f
static const float AS5048A_RES_RAD = AS5048A_RESOLUTION * 0.01745329f;

// 14-bit encoder counts
static const int16_t COUNTS_PER_REV = 16384;
static const int16_t HALF_COUNTS   = COUNTS_PER_REV / 2;

// Globals for tracking
static uint16_t old_result_AS5048A    = 0;
static uint32_t last_sample_time_us   = 0;
static int16_t  diff_AS5048A          = 0;
bool     DIR_AS5048A           = false;
uint16_t res_info_AS5048A      = 0;
static float    last_rpm              = 0.0f;

AS5048A::AS5048A(int chipSelectPin) {
  value = chipSelectPin;
  pinMode(value, OUTPUT);
}

void AS5048A::SPI_setup() {
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
#if F_CPU == 16000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV2);
#elif F_CPU == 72000000UL
  SPI.setClockDivider(SPI_CLOCK_DIV8);
#endif
}

void AS5048A::end_SPI() {
  SPI.end();
}

uint16_t AS5048A::get_raw() {
  uint16_t result1, result2;
  digitalWrite(value, LOW);
  result1 = SPI.transfer(0x00) & 0x3F;
  result1 <<= 8;
  result2 = SPI.transfer(0x00);
  digitalWrite(value, HIGH);
  return (result1 | result2);
}

// Call this as frequently as possible from loop()
void AS5048A::update_info() {
  uint32_t now_us = micros();
  uint16_t new_position = get_raw();

  int16_t delta = (int16_t)new_position - (int16_t)old_result_AS5048A;

  if (delta > HALF_COUNTS)   delta -= COUNTS_PER_REV;
  if (delta < -HALF_COUNTS)  delta += COUNTS_PER_REV;

  uint32_t elapsed_us = now_us - last_sample_time_us;
  if (elapsed_us > 0) {
    float elapsed_s = elapsed_us / 1e6f;
    float revs = (float)delta / COUNTS_PER_REV;
    last_rpm = (revs / elapsed_s) * 60.0f;
  }

  DIR_AS5048A = (delta > 0);
  diff_AS5048A = delta;
  old_result_AS5048A = new_position;
  res_info_AS5048A = new_position;
  last_sample_time_us = now_us;
}

bool AS5048A::get_DIR() {
  return DIR_AS5048A;
}

uint16_t AS5048A::get_pos() {
  return res_info_AS5048A;
}

float AS5048A::get_speed() {
  return fabs(last_rpm);
}
