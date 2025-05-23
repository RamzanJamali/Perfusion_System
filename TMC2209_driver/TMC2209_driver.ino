#include <MobaTools.h>

const byte stepPin = 8;
const byte dirPin = 9;
const byte enablePin = 10;

const int motorStepsPerRev = 200; // Full steps per revolution
const int microsteps = 64;        // Microstepping setting (e.g., 16 for 1/16 microstepping)
const int stepsPerRev = motorStepsPerRev * microsteps;

MoToStepper stepper(stepsPerRev, STEPDIR);

void setup() {
  stepper.attach(stepPin, dirPin);
  stepper.attachEnable(enablePin, 10, LOW); // 10ms delay, LOW to enable
  stepper.setSpeed(1);                     // Speed in RPM
  stepper.setRampLen(50);                   // Ramp length in steps
  stepper.setZero();                        // Set current position as zero

}

void loop() {
  stepper.doSteps(10);
  //stepper.rotate(1); // Rotate clockwise indefinitely
  delay(500);
}
