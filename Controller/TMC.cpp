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
    stepperDriver.enable();
    stepperDriver.setMicrostepsPerStep(256);
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
    runVelocity = flow * 80.89096; // The number 65.77363377 is calculated according to perfusion system parameters. See Formulas.xlsx for further calculations
    // Old -> runVelocity = flow * 65.77363377; // The number 65.77363377 is calculated according to perfusion system parameters. See Formulas.xlsx for further calculations
}