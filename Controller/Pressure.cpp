#include "Pressure.h"


float Pressure(int LOW_PRESSURE, int HIGH_PRESSURE, int raw_low, int raw_high, int raw_pressure) {
	  // Calculate pressure in cmH2O
  float numerator = HIGH_PRESSURE - LOW_PRESSURE;
  float denominator = raw_high - raw_low;
  float m = numerator / denominator;
  float b = LOW_PRESSURE - m * raw_low ;
  float pressure = m * raw_pressure + b; // Pressure in cmH2O
  float p = pressure * 0.73553888361413; // Convert pressure to mmHg
	return p;
}