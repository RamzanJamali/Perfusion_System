#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {

  String ReadEnv = ReadEnvironment();
  Serial.println(ReadEnv);
  delay(2000);
}

String ReadEnvironment(){
  //char result[32];
  float humidity; float temperature; float pressure; float gas;
  if (bme.performReading()) {
    // Read humidity (percent)
    humidity = bme.humidity;
    // Read temperature in Celsius
    temperature = bme.temperature;
    // Read Pressure in hector Pascal hPa
    pressure = (bme.pressure * 0.7500638)/ 100.0;
    // Read Gas in Kohms
    gas = bme.gas_resistance / 1000.0;

    // Check for failed reads
    if (isnan(humidity) || isnan(temperature)) {
      humidity =0; temperature=0; pressure=0; gas = 0;
    }


    String Readings = ("Humidity: " + String(humidity) + "% , Temperature: "+ String(temperature) + " °C , pressure: " + String(pressure) +" mmHg , gas: " + String(gas) + " Kohms");
    //String Readings = (String(humidity) + ", "+ String(temperature) + ", "+ String(pressure) + ", "+ String(gas));
    return Readings;
  }

}