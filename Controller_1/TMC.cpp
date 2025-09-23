#include "TMC.h"

TMC_DRIVER::TMC_DRIVER(uint8_t RX_PIN, uint8_t TX_PIN, uint8_t runCurrentPercent)
    : runCurrentPercent(runCurrentPercent), 
    softSerial(RX_PIN, TX_PIN), 
    runVelocity(0)
    {
        begin();
    }

void TMC_DRIVER::begin() {
    stepperDriver.setup(softSerial);
    stepperDriver.setRunCurrent(runCurrentPercent);
    stepperDriver.enableCoolStep();
    stepperDriver.enableStealthChop();
    stepperDriver.setMicrostepsPerStep(256);
    stepperDriver.enable();
}

void TMC_DRIVER::run() {
    stepperDriver.enable();
    stepperDriver.moveAtVelocity(runVelocity);
}

void TMC_DRIVER::stop() {
    stepperDriver.disable();
    stepperDriver.moveAtVelocity(0);
}

void TMC_DRIVER::flow_rate(float flow) {
    runVelocity = flow * 65.78; // The number 79.5 is calculated according to perfusion system parameters. See Formulas.xlsx and Measurements.xlsx for further calculations
    // Old -> runVelocity = flow * 65.77363377; // The number 65.77363377 is calculated according to perfusion system parameters. See Formulas.xlsx for further calculations
}
