const int sensorPin = A0;   // Analog pin connected to the resistor
const float R_shunt = 250.0; // Shunt resistor value (ohms)

// Adjust based on your sensor's pressure range (e.g., 0–10 bar)
const float pressure_min = 0.0;   // Minimum pressure (bar)
const float pressure_max = 10.0;  // Maximum pressure (bar)

void setup() {
  Serial.begin(9600); // Initialize serial communication
}

void loop() {
  // Read analog value (0–1023 for 0–5V)
  int analogValue = analogRead(sensorPin);
  
  // Convert to voltage (0–5V)
  float voltage = analogValue * (5.0 / 1023.0);
  
  // Convert voltage to current (mA)
  float current_mA = (voltage / R_shunt) * 1000.0;
  
  // Convert current to pressure (linear scaling)
  float pressure = pressure_min + ((current_mA - 4.0) / (20.0 - 4.0)) * (pressure_max - pressure_min);
  
  // Print results
  Serial.print("Voltage: "); Serial.print(voltage, 3); Serial.print(" V | ");
  Serial.print("Current: "); Serial.print(current_mA, 3); Serial.print(" mA | ");
  Serial.print("Pressure: "); Serial.print(pressure, 3); Serial.println(" bar");
  
  delay(1000); // Wait 1 second
}