#include "TMC.h"

TMC_DRIVER::TMC_DRIVER(uint8_t RX_PIN, uint8_t TX_PIN, uint8_t runCurrentPercent)
    : runCurrentPercent(runCurrentPercent), 
    softSerial(RX_PIN, TX_PIN), 
    runVelocity(-0000)
    {
        begin();
    }

void TMC_DRIVER::begin() {
    stepperDriver.setup(softSerial);
    stepperDriver.setRunCurrent(runCurrentPercent);
    stepperDriver.enableCoolStep();
    stepperDriver.enableStealthChop();
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
