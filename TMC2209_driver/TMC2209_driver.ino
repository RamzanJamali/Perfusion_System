#include <MobaTools.h>
#include "AS5048A.h"+

const byte stepPin = 8;
const byte dirPin = 9;
const byte ENABLE_PIN = 5;

const byte CS_PIN = 10;

const int motorStepsPerRev = 200; // Full steps per revolution
const int microsteps = 64;        // Microstepping setting (e.g., 16 for 1/16 microstepping)
const int stepsPerRev = motorStepsPerRev * microsteps;

MoToStepper stepper(stepsPerRev, STEPDIR);

const uint8_t  MAX_SAMPLES = 20;
float         samples[MAX_SAMPLES];
uint8_t       sampleCount = 0;

double rpm;
#define sample 20
// define our CS PIN
AS5048A ABS(CS_PIN);



void setup() {
  
  stepper.attach(stepPin, dirPin);
  //stepper.attachEnable(enablePin, 10, LOW); // 10ms delay, LOW to enable
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable the driver (active-low)
  stepper.setSpeed(1);                     // Speed in RPM
  //stepper.setSpeedSteps(1.17648);
  stepper.setRampLen(1);                   // Ramp length in steps
  //stepper.setZero();                        // Set current position as zero

  ABS.SPI_setup();

  Serial.begin(115200);

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
  ABS.get_info(sample);
  //rpm = abs(averageDelta(ABS.get_speed()));
  rpm = ABS.get_speed();

  Serial.println(rpm);
  delay(100);

}


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
