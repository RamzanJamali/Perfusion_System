#include <MobaTools.h>
#include "AS5048A.h"
#include <SPI.h>

const byte stepPin = 8;
const byte dirPin = 9;
const byte ENABLE_PIN = 5;

const byte CS_PIN = 10;

const int motorStepsPerRev = 200; // Full steps per revolution
const int microsteps = 64;        // Microstepping setting (e.g., 16 for 1/16 microstepping)
const int stepsPerRev = motorStepsPerRev * microsteps;

MoToStepper stepper(stepsPerRev, STEPDIR);


#define sample 80
// define our CS PIN
AS5048A ABS(CS_PIN);

int pos =0;
int  dir_ =0 ;
float sped = 0;


void setup() {
  
  stepper.attach(stepPin, dirPin);
  //stepper.attachEnable(enablePin, 10, LOW); // 10ms delay, LOW to enable
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable the driver (active-low)
  stepper.setSpeed(1);                     // Speed in RPM
  //stepper.setSpeedSteps(1.17648);
  stepper.setRampLen(1);                   // Ramp length in steps
  //stepper.setZero();                        // Set current position as zero


 SPI.begin();

Serial.begin(115200);
// set up SPI 
ABS.SPI_setup();

}

void loop() {
  //digitalWrite(ENABLE_PIN, LOW);
  //stepper.doSteps(-1);
  stepper.rotate(1); // Rotate clockwise indefinitely
  
  //delay(500);
  //stepper.stop(); 
  //digitalWrite(ENABLE_PIN, HIGH);
  //delay(500);
    /// place this in your main loop, and it will update every sample time you defined

  /// place this in your main loop, and it will update every sample time you defined
ABS.get_info(sample);

/// now you can call any of these and get speed, direction or position
dir_  = ABS.get_DIR();
//Serial.println(dir_ );
pos= ABS.get_pos();
//Serial.println(pos);
sped = ABS.get_speed();
//Serial.println(sped,8);
Serial.println(sped);
delay(1000);
}
