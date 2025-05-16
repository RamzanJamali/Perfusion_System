#include <AccelStepper.h>
#define ENABLE_PIN 10
AccelStepper stepper(AccelStepper::DRIVER, 9, 8);


void setup() {
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);           // Enable driver (active low on Pololu board)
  stepper.setMaxSpeed(2);                // Upper speed cap (steps/s)
  stepper.setSpeed(-01.1);            // 0.05 steps/hour ≈ 1.39e-5 steps/sec
}

void loop() {
  stepper.runSpeed();                     // step if interval elapsed
}


