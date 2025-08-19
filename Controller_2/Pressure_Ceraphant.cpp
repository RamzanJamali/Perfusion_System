#include "Pressure_Ceraphant.h"


const float R_shunt = 213.832; // Shunt resistor value (ohms)

// Adjust based on your sensor's pressure range (e.g., 0–1 bar)
const float pressure_min = 0.0;   // Minimum pressure (bar)
const float pressure_max = .10;  // Maximum pressure (bar)

float pressure_cera(int analog_read) {
  // Convert to voltage (0–5V)
  float voltage = analog_read * (5 / 1023.0);
  
  // Convert voltage to current (mA)
  float current_mA = (voltage / R_shunt) * 1000.0;
  
  // Convert current to pressure (linear scaling)
  float _pressure = pressure_min + ((current_mA - 4.0) / (20.0 - 4.0)) * (pressure_max - pressure_min);
  _pressure = _pressure * 750.062;
  
  return _pressure;
}