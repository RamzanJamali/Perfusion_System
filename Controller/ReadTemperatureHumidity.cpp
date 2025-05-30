#include "ReadTemperatureHumidity.h"
#include <adafruit_sensor.h> // Include the Adafruit Sensor library
#include <DHT_U.h>  // Include the DHT Unified Sensor library
#include <DHT.h>    // Include the DHT library

String ReadTempHumidity(byte DHTPIN, byte DHTTYPE) {
  char result[32];
  float humidity; float temperature; float temperatureF;
  DHT dht(DHTPIN, DHTTYPE);
  dht.begin();

   // Read humidity (percent)
  humidity = dht.readHumidity();
  // Read temperature in Celsius
  temperature = dht.readTemperature();
  // Optionally read temperature in Fahrenheit
  temperatureF = dht.readTemperature(true);

  // Check for failed reads
  if (isnan(humidity) || isnan(temperature)) {
   humidity =0; temperature=0; temperatureF=0;
  }
  //return ("Humidity: " + String(humidity) + "% , Temperature: " String(temperature) + " °C , TemperatureF: " + String(temperatureF) +" °F");
   //TempHumid[0] = humidity; TempHumid[1]=temperature; TempHumid[2]=temperatureF;
  snprintf(result, sizeof(result), "%.2f, %.2f, %.2f", humidity, temperature, temperatureF);
}