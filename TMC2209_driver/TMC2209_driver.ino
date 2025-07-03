#include <MobaTools.h>
//#include "AS5048A.h"
#include <SPI.h>

const byte stepPin = 8;
const byte dirPin = 9;
const byte ENABLE_PIN = 5;

const byte CS_PIN = 10;

const int motorStepsPerRev = 200; // Full steps per revolution
const int microsteps = 64;        // Microstepping setting (e.g., 16 for 1/16 microstepping)
const int stepsPerRev = motorStepsPerRev * microsteps;

MoToStepper stepper(stepsPerRev, STEPDIR);

const uint8_t      MAX_SAMPLES       = 10;
const uint16_t     COUNTS_PER_REV    = 1 << 14;      // 16384 counts per 360°
const uint16_t     HALF_COUNTS       = COUNTS_PER_REV / 2;

float       samples[MAX_SAMPLES];
unsigned long timestamps[MAX_SAMPLES];
uint8_t     sampleCount = 0;

double rpm;
//#define sample 20
// define our CS PIN
//AS5048A ABS(CS_PIN);



void setup() {
  
  stepper.attach(stepPin, dirPin);
  //stepper.attachEnable(enablePin, 10, LOW); // 10ms delay, LOW to enable
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable the driver (active-low)
  stepper.setSpeed(1);                     // Speed in RPM
  //stepper.setSpeedSteps(1.17648);
  stepper.setRampLen(1);                   // Ramp length in steps
  //stepper.setZero();                        // Set current position as zero


  Serial.begin(115200);
  initAS5048A();

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
  uint16_t angle = readAS5048A_raw();
  //rpm = abs(averageDelta(ABS.get_speed()));
  rpm = averageRPM(angle);

  Serial.println(rpm);
  delay(100);

}


// call this once in setup()
void initAS5048A() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
}

// read the 14‑bit angle from AS5048A (raw counts 0..16383)
uint16_t readAS5048A_raw() {
  // 1) pull CS low to start transaction
  digitalWrite(CS_PIN, LOW);

  // 2) send two dummy bytes to clock out the 16‑bit response
  uint8_t highByte = SPI.transfer(0x00);
  uint8_t lowByte  = SPI.transfer(0x00);

  // 3) release CS
  digitalWrite(CS_PIN, HIGH);

  // 4) assemble the 14‑bit value (bits 0..13 are valid; top two bits are error/parity)
  uint16_t raw = ((uint16_t)(highByte & 0x3F) << 8) | (uint16_t)lowByte;
  return raw;
}


float averageRPM(uint16_t newCount) {
  unsigned long now = millis();

  // 1) Insert new sample & timestamp (rolling buffer)
  if (sampleCount < MAX_SAMPLES) {
    samples[sampleCount]     = newCount;
    timestamps[sampleCount]  = now;
    sampleCount++;
  } else {
    // drop oldest
    for (uint8_t i = 0; i < MAX_SAMPLES - 1; i++) {
      samples[i]     = samples[i + 1];
      timestamps[i]  = timestamps[i + 1];
    }
    samples[MAX_SAMPLES - 1]     = newCount;
    timestamps[MAX_SAMPLES - 1]  = now;
  }

  // 2) Need at least 2 samples to compute delta
  if (sampleCount < 2) {
    return 0.0f;
  }

  // 3) Sum up count‐deltas (with wrap correction) and time‐deltas
  long        sumCountsDelta = 0;
  unsigned long sumTimeDelta = 0;
  uint8_t     delCount       = sampleCount - 1;

  for (uint8_t i = 0; i < delCount; i++) {
    long diff = long(samples[i + 1]) - long(samples[i]);

    // correct for wrap-around
    if (diff >  HALF_COUNTS)  diff -= COUNTS_PER_REV;
    if (diff < -HALF_COUNTS)  diff += COUNTS_PER_REV;

    sumCountsDelta += diff;
    sumTimeDelta   += (timestamps[i + 1] - timestamps[i]);
  }

  // 4) Avoid division by zero
  if (sumTimeDelta == 0) {
    return 0.0f;
  }

  // 5) Convert to RPM:
  //    (total_counts / counts_per_rev) gives total revolutions;
  //    divide by (sumTimeDelta ms) to get rev/ms;
  //    multiply by 60000 ms/min to get RPM.
  float revolutions    = float(sumCountsDelta) / float(COUNTS_PER_REV);
  float minutesElapsed = float(sumTimeDelta) / 60000.0f;
  return revolutions / minutesElapsed;
}