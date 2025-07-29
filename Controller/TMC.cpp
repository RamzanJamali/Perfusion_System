#include "TMC.h"

TMC_DRIVER::TMC_DRIVER(uint8_t RX_PIN, uint8_t TX_PIN, uint8_t runCurrentPercent)
    : runCurrentPercent(runCurrentPercent), 
    softSerial(RX_PIN, TX_PIN), 
    runVelocity(20000)
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
    if (flow >= 0.1) { // The lowest flow_rate limit
        runVelocity = flow * 65.77363377; // The number 65.77363377 is calculated according to perfusion system parameters. See Formulas.xlsx for further calculations
    }
    else {
        runVelocity = 6.577363377; // microstepping period for flow_rate at 0.1 mL per dau
    }
}