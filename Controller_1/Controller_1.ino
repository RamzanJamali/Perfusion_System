#include "config.h"
#include "TMC.h"
#include "AS5048A.h"
#include "Pressure.h"
#include "PERFUSION.h"
//#include "ReadTemperatureHumidity.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

bool at_end_position = LOW; //define a numeric variable


float desired_flow_rate = 0;
float target_pressure = 15;
int runCurrentPercent = 100;
static double rpm = 0;

bool valve_status = false;
// define our CS PIN 
AS5048A ABS(CS_PIN);

// Initialize variables for pressure sensor
int raw_pressure = 0;         // Variable to store raw sensor reading
static int raw_low = 0;               // low raw sensor reading
static int raw_high = 0;              // high raw sensor reading

// Create perfusion system
Perfusion perfusion(VALVE_PIN, target_pressure); // 
TMC_DRIVER tmc_driver(RX_PIN, TX_PIN, runCurrentPercent);

// commands for arduino to operate. 
String inputString = "";
const char delimiter = ',';
const int maxCommands = 6;
String Commands[maxCommands] = {"IDLE", "500", "2.5", "0", "0", "0"}; // {perfusion_state, pressure, flow_rate, raw_low, raw_high}

String result;
//String data[] = {"1", "90", "45", "45", "45", "2", "CW"}; // Pressure, tilt, gyro x, gyro y, gyro z, Motor Speed, Motor Direction


static uint32_t prev_time = 0;
static uint32_t abs_prev_time = 0;

void setup() {

	Serial.begin(115200);
	//Serial.setTimeout(500);
	perfusion.set_end_position(0); // Set syringe end position
	tmc_driver.begin();

	// BME sensor setup
	bme_setup();

	//pinMode(LED_PIN, OUTPUT);
	pinMode(BUTTON_PIN, INPUT);
	
	// For AS5048A encoder
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

	ABS.SPI_setup();
	ABS.update_info();


	Serial.println("<OK>");

}

void loop() {
	
  if (perfusion.get_valve_state() == HIGH){
        valve_status = true;
      }
	result = ReadTemperatureHumidity();

  raw_pressure = analogRead(PRESSURE_PIN);
  if (raw_low == 0 || raw_high == 0) {
    perfusion.set_current_pressure(raw_pressure);
  }
  else {
    perfusion.set_current_pressure(Pressure(LOW_PRESSURE, HIGH_PRESSURE, raw_low, raw_high, raw_pressure));
  }

	
	if (Serial.available()) {
		inputString = Serial.readStringUntil('\n'); // Read until newline
		inputString.trim();
		//Serial.println(inputString);
		
		if (inputString != Commands[0]+","+Commands[1]+","+Commands[2]+","+Commands[3]+","+Commands[4]+","+Commands[5]) {
			CommandParser(inputString, Commands);
			//Status();
			//Serial.println("<"+Commands[0]+","+Commands[1]+">");
		}
		else{
			//Status();
		}
		
	}

	//for (int x = 0; x<7; x++){
			//Serial.print(Commands[x]);
	//		}
	
	// In next step, these all if conditions will be put in a separate file in a function and only the function will be called here.
	if (Commands[0] == "START_PERFUSION") {
			
      if (perfusion.get_valve_state() == HIGH){
        valve_status = true;
      }
			if (perfusion.get_end_position() == HIGH){
				tmc_driver.stop();
			}
			else {
				perfusion.start_perfusion();
				tmc_driver.run();
			}


	} else if (Commands[0] == "PAUSE_PERFUSION") {
		  perfusion.pause_perfusion();
			tmc_driver.stop();

	} else if (Commands[0] == "CONTINUE_PERFUSION"){
			
      if (perfusion.get_valve_state() == 1){
        valve_status = true;
      }
			if (perfusion.get_end_position() == HIGH){
				tmc_driver.stop();
			}
			else {
				perfusion.start_perfusion();
				tmc_driver.run();
			}


	} else if (Commands[0] == "END_PERFUSION") {
		  perfusion.end_perfusion();
			tmc_driver.stop();
	} else {
		}

	// Secondary commands
	if (Commands[1] != "1"){
		perfusion.set_target_pressure(Commands[1].toFloat());
	} else {}

	if(Commands[2].toFloat() != desired_flow_rate){
		desired_flow_rate = Commands[2].toFloat();
		tmc_driver.flow_rate(desired_flow_rate);
	} 
	else {
		}
	if (desired_flow_rate < -1 && perfusion.get_state() == 0) {
		tmc_driver.run();
	}
	else if (perfusion.get_state() == 0) {
		tmc_driver.stop();
	}


	if ((Commands[3].toFloat() != -1) && perfusion.get_state() == 0) {
			raw_low = Commands[3].toFloat();
	}

	if ((Commands[4].toFloat() != -1) && perfusion.get_state() == 0) {
		raw_high = Commands[4].toFloat();
	}
	
	if (perfusion.get_state() == 0) {
		if (Commands[5].toInt() == 1) {
    	if (perfusion.get_valve_state() == 0){
      perfusion.opened_valve();
      
      } else {}

		} else if (Commands[5].toInt() == 0){
			perfusion.closed_valve();
		} else {}
	}

  raw_pressure = analogRead(PRESSURE_PIN);
	if (raw_low == 0 || raw_high == 0) {
		perfusion.set_current_pressure(raw_pressure);
	}
	else {
		perfusion.set_current_pressure(Pressure(LOW_PRESSURE, HIGH_PRESSURE, raw_low, raw_high, raw_pressure));
	}
  


  uint32_t abs_current_time = millis();
  if (abs_current_time - abs_prev_time >= 10000) {
    ABS.update_info();
    rpm = ABS.get_speed();
    
    abs_prev_time = abs_current_time;
  }

  if (perfusion.get_valve_state() == HIGH){
        valve_status = true;
      }
	uint32_t current_time = millis();
	if (current_time - prev_time > 990) { 
	  Status();
		if (perfusion.get_state() == 1) {
			valve_status = perfusion.get_valve_state();
		}
		prev_time = current_time;
	}


	//perfusion.update_data(data);
	//delay(999);
	//Status();


 float a = perfusion.get_current_pressure();
 float b = perfusion.get_target_pressure();
 b = b * 1.5;


 if (a > b) {
    at_end_position = HIGH;
    perfusion.set_end_position(at_end_position);
 } else {
  at_end_position = digitalRead(BUTTON_PIN);
  perfusion.set_end_position(at_end_position);
  }
 
}

void CommandParser(String inputString, String *Commands) {
	int index = 0;

	while (inputString.length() > 0 && index < maxCommands) {
		int delimiterIndex = inputString.indexOf(delimiter);
		if (delimiterIndex == -1) {
			Commands[index++] = inputString;
			break;
		} else {
			Commands[index++] = inputString.substring(0, delimiterIndex);
			inputString = inputString.substring(delimiterIndex + 1);
		}
	}
}


void Status(){
	// perfusion.get_motor_speed() will be replaced by flow rate.
	Serial.println("<"+ String(perfusion.get_state()) + ", " + String(valve_status) +", "+ result + ", " + perfusion.get_current_pressure()+ ", "+ perfusion.get_target_pressure() + ", "+ String(rpm,4) +">");
}


void bme_setup(){
  bme.begin();
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}


String ReadTemperatureHumidity() {
  float humidity, temperature, pressure;
  uint32_t gasRes;  // gas resistance in Ω

  if (!bme.performReading()) {
    // failed read
    return String("0, 0, 0, 0, 0");
  }

  // Sensor readings
  humidity    = bme.humidity;                          // % RH
  temperature = bme.temperature;                       // °C
  pressure    = (bme.pressure * 0.7500638) / 100.0;    // mmHg
  gasRes      = bme.gas_resistance;                    // Ω

  // 1. Gas score (0–75)
  const float R_min = 50.0;        // Ω (very polluted)
  const float R_max = 50000.0;     // Ω (clean air)
  float gas_score = (R_max - gasRes) / (R_max - R_min) * 75.0;
  gas_score = constrain(gas_score, 0.0, 75.0);

  // 2. Humidity score (0–25)
  float hum_score;
  if (humidity < 40.0) {
    hum_score = (40.0 - humidity) * (25.0 / 40.0);
  } else {
    hum_score = (humidity - 40.0) * (25.0 / 60.0);
  }
  hum_score = constrain(hum_score, 0.0, 25.0);

  // 3. Combine into IAQ (0–500)
  float raw_iaq   = gas_score + hum_score;   // 0–100%
  float IAQ_index = raw_iaq * 5.0;           // scale to 0–500

  // Build CSV string: humidity, temperature, pressure, gas (kΩ), IAQ
  String Readings = 
    String(humidity, 1) + ", " +
    String(temperature, 1) + ", " +
    String(pressure, 1) + ", " +
    String(IAQ_index, 0);

  return Readings;
}


/*

float averageDelta(float newSample) {
  // 1) Insert newSample into our rolling buffer
  if (sampleCount < MAX_SAMPLES) {
    samples[sampleCount++] = newSample;
  } else {
    // shift left, drop samples[0]
    for (uint8_t i = 0; i < MAX_SAMPLES - 1; i++) {
      samples[i] = samples[i + 1];
    }
    samples[MAX_SAMPLES - 1] = newSample;
  }

  // 2) If we don't yet have at least two samples, no delta to compute
  if (sampleCount < 2) {
    return 0.0;
  }

  // 3) Sum up differences between consecutive samples
  float sumDeltas = 0.0;
  uint8_t delCount = sampleCount - 1;
  for (uint8_t i = 0; i < delCount; i++) {
    sumDeltas += (samples[i + 1] - samples[i]);
  }

  // 4) Return average difference
  return sumDeltas / delCount;
}
*/
