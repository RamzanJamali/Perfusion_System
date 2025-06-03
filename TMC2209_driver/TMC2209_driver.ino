#include <MobaTools.h>

const byte stepPin = 8;
const byte dirPin = 9;
const int ENABLE_PIN = 10;

const int motorStepsPerRev = 200; // Full steps per revolution
const int microsteps = 64;        // Microstepping setting (e.g., 16 for 1/16 microstepping)
const int stepsPerRev = motorStepsPerRev * microsteps;

MoToStepper stepper(stepsPerRev, STEPDIR);

void setup() {
  stepper.attach(stepPin, dirPin);
  //stepper.attachEnable(enablePin, 10, LOW); // 10ms delay, LOW to enable
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable the driver (active-low)
  //stepper.setSpeed(1);                     // Speed in RPM
  stepper.setSpeedSteps(1.17648);
  stepper.setRampLen(1);                   // Ramp length in steps
  //stepper.setZero();                        // Set current position as zero

}

void loop() {
  //digitalWrite(ENABLE_PIN, LOW);
  //stepper.doSteps(-1);
  stepper.rotate(1); // Rotate clockwise indefinitely
  
  //delay(500);
  //stepper.stop(); 
  //digitalWrite(ENABLE_PIN, HIGH);
  //delay(500);

}
