// Include necessary libraries for LCD, DHT sensor, and BME680 sensor
#include <LiquidCrystal_I2C.h>;
#include <Wire.h>;
#include "DHT.h" ;
#include <seeed_bme680.h> ;

// LCD setup
LiquidCrystal_I2C lcd (0x27,20,4); // Set the LCD address to 0x27 for a 20 chars and 4 line display

// DHT22 sensor setup
#define DHTPIN 2              // Define the pin where the DHT22 sensor is connected
#define DHTTYPE DHT22         // Specify the DHT sensor type
DHT dht(DHTPIN, DHTTYPE);

// BME680 sensor setup
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define IIC_ADDR uint8_t(0x76)
Seeed_BME680 bme680(IIC_ADDR); /* IIC PROTOCOL */ // Initialize BME680 sensor with I2C protocol

// Initialize variables for pressure sensor
const int sensorPin1 = A0;    // Analog pin for pressure sensor
int sensorValue1 = 0;         // Variable to store raw sensor reading
int low = 0;                  // low pressure in cmH2O
int high = 0;                 // high pressure in cmH2O
int rawLow = 0;               // low raw sensor reading
int rawHigh = 0;              // high raw sensor reading

void setup() {
 Serial.begin(9600);          // Initialize serial communication
 dht.begin();                 // Initialize DHT sensor

 // Set calibration values for pressure sensor
 high = 10 ;
 low = 5 ;
 rawLow = 46;
 rawHigh = 96;
 

 // Initialize LCD
 lcd.init();
 lcd.backlight();

 // Set LCD labels
 lcd.setCursor(0,0);
 lcd.print("raw:");     // Raw sensor value
 
 lcd.setCursor(10,0);
 lcd.print("lraw:");     // low raw sensor reading 
 
 lcd.setCursor(0,1);
 lcd.print("tDHT:");     // Temperature from DHT
 lcd.setCursor(9,1);
 lcd.write(223); 
 lcd.print("C"); 

 lcd.setCursor(12,1);
 lcd.print("hDHT:");     // Humidity from DHT
 lcd.setCursor(19,1);
 lcd.print("%"); 
 
 lcd.setCursor(0,2);
 lcd.print ("p Cornea:");    // liquid pressure Cornea
 lcd.setCursor(16,2);
 lcd.print ("mmHg");

 lcd.setCursor(0,3);
 lcd.print ("airp BME:");    // air Pressure
 lcd.setCursor(16,3);
 lcd.print ("kPa"); 
 
 // Initialize BME680 sensor
  while (!bme680.init()) {
    Serial.println("bme680 init failed ! can't find device!");
    delay(10000);
  }
}

void loop() {

  // Read pressure Sensor
  sensorValue1 = analogRead(sensorPin1);

  // Calculate pressure in cmH2O
  float numerator = high - low;
  float denominator = rawHigh - rawLow;
  float m = numerator / denominator;
  float b = low - m * rawLow ;
  float pressure = m * sensorValue1 + b; // Pressure in cmH2O
  float p = pressure * 0.73553888361413; // Convert pressure to mmHg

  // Read temperature and humidity from DHT22
  float h = dht.readHumidity();
  int humidity = round(h);               // Round humidity to nearest integer
  float t = dht.readTemperature();
  char tempString[6];
  dtostrf(t, 4, 1, tempString);          // Convert temperature to string with 1 decimal place

  // Check for sensor reading failures
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Read BME680 sensor data
  if (bme680.read_sensor_data()) {
    Serial.println("Failed to perform reading from BME680 sensor!");
    return;
  }

  // Store BME680 sensor readings
  float BMEt = (bme680.sensor_result_value.temperature);
  float BMEh = (bme680.sensor_result_value.humidity);
  float BMEp = (bme680.sensor_result_value.pressure);
  float BMEg = (bme680.sensor_result_value.gas);

  // Print measurements to serial monitor 
  String values = String(rawLow) + ", " + 
                  String(sensorValue1) + ", " +  
                  String(p) + ", " +
                  String(t) + ", " +  
                  String(h) + ", " +
                  String(BMEt) + ", " +
                  String(BMEh) + ", " +
                  String(BMEp) + ", " +
                  String(BMEg);
                  
  Serial.println(values);
  
  // Print measurements to LCD
  lcd.setCursor(4,0);
  lcd.print(sensorValue1);
  lcd.setCursor(16,0);
  lcd.print (rawLow);
  
  lcd.setCursor(5,1);
  lcd.print(tempString);
  lcd.setCursor(17,1);
  lcd.print(humidity);

  lcd.setCursor(10,2);
  lcd.print(p);

  lcd.setCursor(10,3);
  lcd.print(BMEp/ 1000.0);

  // Wait before taking the next measurement
  delay(500);
}