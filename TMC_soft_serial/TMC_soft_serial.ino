#include <TMC2209.h>
#include "AS5048A.h"

// SoftwareSerial can be used on Arduino boards without HardwareSerial ports,
// such as the Uno, Nano, and Mini.
//
// See this reference for more details:
// https://www.arduino.cc/reference/en/language/functions/communication/serial/

// Software serial ports should only be used for unidirectional communication
// The RX pin does not need to be connected, but it must be specified when
// creating an instance of a SoftwareSerial object
const uint8_t RX_PIN = 4;
const uint8_t TX_PIN = 5;
const uint8_t CS_PIN = 10;

SoftwareSerial soft_serial(RX_PIN, TX_PIN);

const int32_t RUN_VELOCITY = 164.434;
const int32_t STOP_VELOCITY = 0;
const int RUN_DURATION = 2000;
const int STOP_DURATION = 1000;
// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage
const uint8_t RUN_CURRENT_PERCENT = 30;


// Instantiate TMC2209
TMC2209 stepper_driver;
bool invert_direction = true;


double rpm;
// define our CS PIN 
AS5048A ABS(CS_PIN);

static uint32_t prev_time = 0;
static uint32_t start_time = 0;
static uint16_t prev_raw = 0;
static int32_t  half_range = 16384 / 2;
static int64_t  abs_counts = 0;    // signed total counts since start


void setup()
{
  Serial.begin(115200);
  stepper_driver.setup(soft_serial);

  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enableCoolStep();
  stepper_driver.enable();

  stepper_driver.setMicrostepsPerStep(256); // Set microstepping
  	// For AS5048A encoder
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

	ABS.SPI_setup();
  ABS.update_info();
  // 1) Read the very first raw position and treat it as "zero"
  prev_raw = ABS.get_pos();  

}

void loop()
{
  //stepper_driver.moveAtVelocity(STOP_VELOCITY);
  //delay(STOP_DURATION);
  /*if (invert_direction)
  {
    stepper_driver.enableInverseMotorDirection();
  }
  else
  {
    stepper_driver.disableInverseMotorDirection();
  }
  invert_direction = not invert_direction;
*/stepper_driver.enable();
  stepper_driver.moveAtVelocity(RUN_VELOCITY);

  //delay(RUN_DURATION);

  	/// place this in your main loop, and it will update every sample time you defined
  uint32_t current_time = millis();
    
	if (current_time - prev_time > 5000) {
		prev_time = current_time;
		ABS.update_info();

    rpm = ABS.get_speed();
    //Serial.println(rpm, 4);


    uint16_t current_raw = ABS.get_pos();       // 0…16383
    int32_t  delta       = int32_t(current_raw) - int32_t(prev_raw);

    // wrap detection
    if      (delta >  half_range) delta -= 16384;
    else if (delta < -half_range) delta += 16384;

    abs_counts += delta;    // accumulate true signed counts
    prev_raw   = current_raw;

    // compute total rotations and angle
    float rotations = float(abs_counts) / 16384.0;  
    // fractional part for angle:
    int32_t frac_counts = abs_counts % 16384;
    //if (frac_counts < 0) frac_counts += 16384;   // keep 0…16383
    //float angle_deg = (frac_counts / 16383.0) * 360.0;

    double rpm = ABS.get_speed();

    Serial.print("RPM: ");
    Serial.print(rpm, 4);
    Serial.print(" | Turns: ");
    Serial.print(abs(rotations), 4);
    double mL = rotations/20 * 0.2520;
    Serial.print(" | mL: ");
    Serial.print(abs(mL), 5);
    Serial.print("  | Time elapsed: ");
    double time_elapsed = (millis() - start_time)/(1000); // Time in minutes
    Serial.print(time_elapsed/60);
    Serial.println(".");
    //Serial.print(" | Angle: ");
    //Serial.println(angle_deg, 2);
    
    
	}
	
  //delay(3000);
  //stepper_driver.disable();
  //delay(3000);
}