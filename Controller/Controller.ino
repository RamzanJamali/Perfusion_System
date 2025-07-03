#include "config.h"
#include <MobaTools.h>
#include "Perfusion.h"
#include "AS5048A.h"
#include "Pressure.h"
//#include "ReadTemperatureHumidity.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


constexpr uint8_t DHT_TYPE = DHT22;     // Type of sensor: DHT22 (AM2302 / AM2321)
DHT dht(DHT_PIN, DHT_TYPE);

int at_end_position; //define a numeric variable


float desired_flow_rate = 1.7;
double rpm;
#define sample 1000
// define our CS PIN
AS5048A ABS(CS_PIN);

// Initialize variables for pressure sensor
int raw_pressure = 0;         // Variable to store raw sensor reading
static int raw_low = 0;               // low raw sensor reading
static int raw_high = 0;              // high raw sensor reading

// Create perfusion system
Perfusion perfusion(STEP_PIN, DIR_PIN, ENABLE_PIN, VALVE_PIN, 
500, desired_flow_rate, 200, 64); // Target pressure 500, initial speed 1

// commands for arduino to operate. 
String inputString = "";
const char delimiter = ',';
const int maxCommands = 5;
String Commands[maxCommands] = {"IDLE", "500", "1.7", "0", "0"}; // {perfusion_state, pressure, flow_rate, raw_low, raw_high}

String result;
String data[] = {"1", "90", "45", "45", "45", "2", "CW"}; // Pressure, tilt, gyro x, gyro y, gyro z, Motor Speed, Motor Direction

void setup() {

	Serial.begin(115200);
	//Serial.setTimeout(500);
	perfusion.set_end_position(0); // Set syringe end position

	//pinMode(LED_PIN, OUTPUT);
	pinMode(BUTTON_PIN, INPUT);
	
	// For AS5048A encoder
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

	ABS.SPI_setup();

  dht.begin();
	Serial.println("<OK>");

}

void loop() {
  raw_pressure = analogRead(PRESSURE_PIN);
  if (raw_low == 0 || raw_high == 0) {
    perfusion.set_current_pressure(raw_pressure);
  }
  else {
    perfusion.set_current_pressure(Pressure(LOW_PRESSURE, HIGH_PRESSURE, raw_low, raw_high, raw_pressure));
  }
  perfusion.open_valve();
	at_end_position = digitalRead(BUTTON_PIN);
	perfusion.set_end_position(at_end_position);

	

/*
	if(at_end_position == HIGH) { // turn on LED when sensor is blocked
		digitalWrite(LED_PIN,HIGH);
	}
	else {
		digitalWrite(LED_PIN,LOW);
	}
*/

	if (Serial.available()) {
		inputString = Serial.readStringUntil('\n'); // Read until newline
		inputString.trim();
		Serial.println(inputString);
		/*if (inputString == Commands[0]+","+Commands[1]){
      Status();
    }
    else if */
		if (inputString != Commands[0]+","+Commands[1]+","+Commands[2]) {
			CommandParser(inputString, Commands);
			//Status();
			//Serial.println("<"+Commands[0]+","+Commands[1]+">");
		}
		else{
			//Status();
		}
	}

	// In next step, these all if conditions will be put in a separate file in a function and only the function will be called here.
	if (Commands[0] == "START_PERFUSION") {
		perfusion.start_perfusion();

	} else if (Commands[0] == "PAUSE_PERFUSION") {
		perfusion.pause_perfusion();

	} else if (Commands[0] == "CONTINUE_PERFUSION"){
		perfusion.start_perfusion();

	} else if (Commands[0] == "END_PERFUSION") {
		perfusion.end_perfusion();
	} else {
		}

	// Secondary commands
	if (Commands[1] != "1"){
		perfusion.set_target_pressure(Commands[1].toFloat());
	} else {}

	if(Commands[2].toFloat() != desired_flow_rate){
		desired_flow_rate = Commands[2].toFloat();
		perfusion.set_flow_rate(desired_flow_rate);
		//desired_motor_speed = Commands[2].toFloat();
		//perfusion.set_speed(desired_motor_speed);
	} else {
		}

	if ((Commands[3].toFloat() != -1) && perfusion.get_state() == 0) {
			raw_low = Commands[3].toFloat();
	}

	if ((Commands[4].toFloat() != -1) && perfusion.get_state() == 0) {
		raw_high = Commands[4].toFloat();
	}
	
	/// place this in your main loop, and it will update every sample time you defined
	ABS.get_info(sample);
	rpm = ABS.get_pos();
	perfusion.set_current_motor_speed(rpm);

  raw_pressure = analogRead(PRESSURE_PIN);
	if (raw_low == 0 || raw_high == 0) {
		perfusion.set_current_pressure(raw_pressure);
	}
	else {
		perfusion.set_current_pressure(Pressure(LOW_PRESSURE, HIGH_PRESSURE, raw_low, raw_high, raw_pressure));
	}
 
  perfusion.open_valve();
  
	//Serial.println("<"+Commands[0]+", " +Commands[1]+", " + Commands[2]+", " +perfusion.get_steps_per_second()+">"); // In future get_steps_per_second() should be replaced by flow_rate calculated using motor_speed provided by sensor.
	result = ReadTemperatureHumidity();
	Status();
	//perfusion.update_data(data);
	delay(1000);
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
	Serial.println("<"+ String(perfusion.get_state()) + ", " + String(perfusion.get_valve_state()) +", "+ result + ", " + perfusion.get_current_pressure()+ ", "+ perfusion.get_target_pressure() + ", "+ String(rpm,6) + ", " + data[1] + ", " + data[2] +", "+ data[3] +", "+ data[4] +">");
}


String ReadTemperatureHumidity(){
  //char result[32];
  float humidity; float temperature; float temperatureF;

  // Read humidity (percent)
  humidity = dht.readHumidity();
  // Read temperature in Celsius
  temperature = dht.readTemperature();
  // Optionally read temperature in Fahrenheit
  //temperatureF = dht.readTemperature(true);

  // Check for failed reads
  if (isnan(humidity) || isnan(temperature)) {
  	humidity =0; temperature=0; temperatureF=0;
  }
	//String humidtemp = ("Humidity: " + String(humidity) + "% , Temperature: "+ String(temperature) + " °C , TemperatureF: " + String(temperatureF) +" °F");
	String humidtemp = (String(humidity) + ", "+ String(temperature));
	return humidtemp;

}
