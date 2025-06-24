#ifndef AS5048A_H
#define AS5048A_H

#include <SPI.h>

void SPI_begin();

uint16_t read_raw_angle(const uint8_t CS_PIN);

double raw_to_radians(uint16_t raw);

double speed_in_rpm(const uint8_t CS_PIN, double rpm);

#endif