#include <Arduino.h>
#include <TimerTwo.h>
#include "BasicStepperDriver.h"

// Motor & driver parameters
#define MOTOR_STEPS    200        // full‑steps per revolution
#define NATIVE_MICRO   32         // DRV8825 wiring: 1/32 microstep
#define TARGET_MICRO   256        // desired effective microstep
#define DIR_PIN        8
#define STEP_PIN       11         // OC2A on ATmega328P (Arduino D11)

// User settings
const float rpm = 1.0;            // target speed

// Stepper driver (we use it only for DIR and ENABLE)
BasicStepperDriver stepper(MOTOR_STEPS, DIR_PIN, STEP_PIN);

// Compute the period (microseconds) between toggles for TimerTwo
// TimerTwo toggles pin each interrupt, so two interrupts = one full pulse
unsigned long calcPeriodMicros(float rpm) {
  // pulses per second = (MOTOR_STEPS * TARGET_MICRO * rpm) / 60
  float pulsesPerSec = MOTOR_STEPS * TARGET_MICRO * rpm / 60.0;
  // interrupts per second = pulsesPerSec * 2
  float intsPerSec = pulsesPerSec * 2.0;
  // period in microseconds = 1e6 / intsPerSec
  return (unsigned long)(1e6 / intsPerSec);
}

// ISR called by TimerTwo on each compare match:
void stepISR() {
  // toggle the STEP pin
  // (TimerTwo library uses digitalWriteFast if available)
  digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
}

void setup() {
  // 1. BasicStepperDriver: set DIR pin mode & enable output
  stepper.begin(rpm, NATIVE_MICRO);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);    // CW; set HIGH for CCW

  // 2. Prepare STEP pin
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);   // start low so the first toggle goes HIGH

  // 3. Initialize TimerTwo
  //    period in microseconds between ISR calls
  unsigned long period = calcPeriodMicros(rpm);
  TimerTwo::initialize(period);
  TimerTwo::attachInterrupt(stepISR);

  // 4. Enable timer (starts firing interrupts)
  TimerTwo::start();
}

void loop() {
  // nothing to do in loop: TimerTwo toggles STEP automatically
}
