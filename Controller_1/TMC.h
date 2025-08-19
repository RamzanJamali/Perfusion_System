// TMC2209Driver.h
#ifndef TMC_H
#define TMC_H

#include <TMC2209.h>
#include <SoftwareSerial.h>

class TMC_DRIVER {
public:
    TMC_DRIVER(uint8_t RX_PIN, uint8_t TX_PIN, uint8_t runCurrentPercent);
    void begin();
    void run();
    void stop();
    void flow_rate(float flow);

private:
    SoftwareSerial softSerial;
    TMC2209 stepperDriver;
    int32_t runVelocity;
    uint8_t runCurrentPercent;
};
#endif // TMC2209_DRIVER_H
